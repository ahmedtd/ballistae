#ifndef LIBBALLISTAE_IMAGE_HH
#define LIBBALLISTAE_IMAGE_HH

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <cassert>
#include <climits>
#include <cstddef>

#include <numeric>
#include <sstream>
#include <type_traits>
#include <vector>

namespace ballistae
{

namespace image_detail
{

}

template<class Elt>
class image
{
    std::vector<size_t> idx_lims;
    std::vector<Elt> storage;

public:

    image(
        std::initializer_list<std::size_t> idx_lims_in,
        Elt fill = Elt(0)
    )
        : idx_lims(idx_lims_in),
          storage(
              std::accumulate(
                  idx_lims.begin(),
                  idx_lims.end(),
                  1,
                  [](size_t a, size_t b){return a*b;}
              ),
              fill
          )
    {
    }

    template<class... ArgTail>
    image& resize(ArgTail... arg_tail)
    {
        idx_lims = {arg_tail...};
        storage.resize(
            std::accumulate(
                idx_lims.begin(),
                idx_lims.end(),
                1,
                [](size_t a, size_t b){return a*b;}
            )
        );
    }

    template<class... ArgTail>
    Elt& index(ArgTail... arg_tail)
    {
        std::array<size_t, sizeof...(arg_tail)> indices = {{static_cast<size_t>(arg_tail)...}};
        size_t stride = 1;
        size_t comp_index = 0;
        for(size_t i = indices.size(); i != 0; --i)
        {
            comp_index += indices[i-1] * stride;
            stride     *= idx_lims[i-1];
        }

        return storage[comp_index];
    }

    template<class... ArgTail>
    const Elt& index(ArgTail... arg_tail) const
    {
        return const_cast<image<Elt>*>(this)->index(arg_tail...);
    }

    template<class... ArgTail>
    Elt& operator()(ArgTail... arg_tail)
    {
        return index(arg_tail...);
    }

    template<class... ArgTail>
    const Elt& operator()(ArgTail... arg_tail) const
    {
        return index(arg_tail...);
    }

    size_t size() const
    {
        return storage.size();
    }

    size_t n_indices() const
    {
        return idx_lims.size();
    }

    size_t idx_lim(size_t which_idx) const
    {
        return idx_lims[which_idx];
    }
};

template<class Elt>
int write_pfm(const image<Elt> &img, const std::string &file)
{
    static_assert(
        sizeof(uint32_t) == sizeof(float),
        "Assume that a float is an IEEE754 b32 floating-point number."
    );

    static_assert(
        CHAR_BIT == 8,
        "Assume that a char has 8 bits."
    );

    // pfm can only handle a 3-index image where the third index is 1
    // (greyscale) or 3 (color triples).
    assert(img.n_indices() == 3);
    assert(img.idx_lim(2) == 1 || img.idx_lim(2) == 3);

    size_t img_rows = img.idx_lim(0);
    size_t img_cols = img.idx_lim(1);
    size_t img_samp = img.idx_lim(2);

    // Create the header.  We will always write a big-endian pfm image.
    std::string header;
    {
        std::stringstream header_stream;
        header_stream << "P" << (img.idx_lim(2) == 1 ? "f" : "F") << "\n"
                      << img_cols << " " << img_rows << "\n"
                      << "1.0\n";
        header = header_stream.str();
    }

    size_t n_data = img_rows * img_cols * img_samp;
    size_t data_size = n_data * sizeof(Elt);
    size_t file_size = header.size() + data_size;

    void *buf = nullptr;

    int fd = open(
        file.c_str(),
        O_RDWR | O_CREAT,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
    );

    if(fd == -1)
        goto cleanup_exit_error;

    if(ftruncate(fd, file_size) != 0)
        goto cleanup_close_file;

    buf = mmap(
        nullptr,
        file_size,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fd,
        0
    );

    if(buf == MAP_FAILED)
        goto cleanup_close_file;

    if(madvise(buf, file_size, MADV_SEQUENTIAL | MADV_WILLNEED) == -1)
        goto cleanup_unmap_file;

    // Copy header to buf.
    {
        uint8_t *out_src = (uint8_t*) buf;
        out_src = std::copy(header.begin(), header.end(), out_src);

        size_t row_i = img_rows;
        do
        {
            --row_i;

            const Elt *row_src = &(img(row_i, 0, 0));
            const Elt *row_lim = row_src + img_cols * img_samp;

            for(; row_src != row_lim; ++row_src)
            {
                // Using a union in this way is technically undefined behavior,
                // but it's supported by GCC and clang.
                union { float flt; uint32_t bit; } a;

                // Convert the current sample to a float, and get its bytes.
                a.flt = *row_src;

                // Write the current sample to the file (big-endian).
                out_src[0] = a.bit >> 24;
                out_src[1] = a.bit >> 16;
                out_src[2] = a.bit >>  8;
                out_src[3] = a.bit >>  0;

                out_src += 4;
            }
        }
        while(row_i != 0);
    }

    if(munmap(buf, file_size) == -1)
        goto cleanup_close_file;

    close(fd);
    return 0;

 cleanup_unmap_file:
    munmap(buf, file_size);
 cleanup_close_file:
    close(fd);
 cleanup_exit_error:
    return -1;
}

}

#endif
