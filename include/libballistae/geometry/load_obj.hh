#ifndef BALLISTAE_GEOMETRY_LOAD_OBJ_HH
#define BALLISTAE_GEOMETRY_LOAD_OBJ_HH

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cinttypes>

#include <algorithm>
#include <array>
#include <string>
#include <tuple>

#include "include/libballistae/geometry/tri_mesh.hh"

namespace ballistae
{

constexpr int OBJ_ERRC_NONE = 0;
constexpr int OBJ_ERRC_FILE_NOT_OPENABLE = 1;
constexpr int OBJ_ERRC_FILE_NOT_LOADABLE = 2;
constexpr int OBJ_ERRC_PARSE_ERROR = 3;
constexpr int OBJ_ERRC_INSANE = 4;

/// Test if [a_src, a_lim) begins with [b_src, b_lim)
template<class ItA, class ItB>
bool begins_with(
    ItA a_src, ItA a_lim,
    ItB b_src, ItB b_lim
)
{
    while(a_src != a_lim && b_src != b_lim && *a_src == *b_src)
    {
        ++a_src;
        ++b_src;
    }

    return b_src == b_lim;
}

std::tuple<bool, const char*> parse_literal(
    const char *src,
    const char *lim,
    const char *const lit
)
{
    using std::strlen;
    if(begins_with(src, lim, lit, lit + strlen(lit)))
        return std::make_tuple(true, src + strlen(lit));
    else
        return std::make_tuple(false, src);
}

std::tuple<bool, const char*, size_t> parse_size_t(
    const char *src,
    const char *lim
)
{
    const char *cur = src;

    char *strtoumax_end;
    uintmax_t parsed = std::strtoumax(cur, &strtoumax_end, 10);
    cur = strtoumax_end;
    if(parsed < std::numeric_limits<size_t>::min()
       || parsed > std::numeric_limits<size_t>::max()
       || cur == src)
        return std::make_tuple(false, src, std::numeric_limits<size_t>::max());
    else
        return std::make_tuple(true, cur, parsed);
}

std::tuple<bool, const char*, double> parse_double(
    const char *src,
    const char *lim
)
{
    const char *cur = src;

    char *strtod_end;
    double parsed = std::strtod(cur, &strtod_end);
    cur = strtod_end;

    if(cur == src)
        return std::make_tuple(false, src, 0.0);
    else
        return std::make_tuple(true, cur, parsed);
}

/// Parse a newline sequence.
///
/// Detects any common one-or-two-byte newline sequence, as well as end of the
/// file.
std::tuple<bool, const char*> parse_newline(const char *src, const char *lim)
{
    const char *cur = src;

    // We allow arbitrary blank space at the end of the line.
    while(cur != lim && (*cur == 0x9 || *cur == 0x20))
        ++cur;

    // This will handle pretty much any newline sequence we can expect to see.

    if(cur != lim && *cur == 0xd)
        ++cur;

    if(cur != lim && *cur == 0xa)
        ++cur;

    if(cur == src && cur != lim)
        return std::make_tuple(false, src);
    else
        return std::make_tuple(true, cur);
}

std::tuple<bool, const char*> parse_blank_line(const char *src, const char *lim)
{
    const char *cur = src;

    while(cur != lim && (*cur == 0x9 || *cur == 0x20))
        ++cur;

    bool success;
    std::tie(success, cur) = parse_newline(cur, lim);
    if(! success)
        return std::make_tuple(false, src);

    return std::make_tuple(true, cur);
}

std::tuple<bool, const char*> parse_comment_line(const char *src, const char *lim)
{
    const char *cur = src;

    bool success;
    std::tie(success, cur) = parse_literal(cur, lim, "#");
    if(! success)
        return std::make_tuple(false, src);

    // Advance to something that could be a newline sequence.
    while(cur != lim && *cur != 0xa && *cur != 0xd)
        ++cur;

    std::tie(success, cur) = parse_newline(cur, lim);
    if(! success)
        return std::make_tuple(false, src);

    return std::make_tuple(true, cur);
}

std::tuple<bool, const char*> parse_texcoord_line(
   const char *src,
   const char *lim,
   tri_mesh &the_mesh
)
{
    const char *cur = src;

    bool success;
    std::tie(success, cur) = parse_literal(cur, lim, "vt");
    if(! success)
        return std::make_tuple(false, src);

    double u, v;
    std::tie(success, cur, u) = parse_double(cur, lim);
    if(! success)
        return std::make_tuple(false, src);
    std::tie(success, cur, v) = parse_double(cur, lim);
    if(! success)
        return std::make_tuple(false, src);

    std::tie(success, cur) = parse_newline(cur, lim);
    if(! success)
        return std::make_tuple(false, src);

    the_mesh.m.push_back({u, v});

    return std::make_tuple(true, cur);
}

std::tuple<bool, const char*> parse_normal_line(
    const char *src,
    const char *lim,
    tri_mesh &the_mesh,
    bool swapyz
)
{
    const char *cur = src;

    bool success;
    std::tie(success, cur) = parse_literal(cur, lim, "vn");
    if(! success)
        return std::make_tuple(false, src);

    double x, y, z;
    std::tie(success, cur, x) = parse_double(cur, lim);
    if(! success)
        return std::make_tuple(false, src);
    std::tie(success, cur, y) = parse_double(cur, lim);
    if(! success)
        return std::make_tuple(false, src);
    std::tie(success, cur, z) = parse_double(cur, lim);
    if(! success)
        return std::make_tuple(false, src);

    std::tie(success, cur) = parse_newline(cur, lim);
    if(! success)
        return std::make_tuple(false, src);

    if(swapyz)
    {
        using std::swap;
        swap(y, z);
    }

    the_mesh.n.push_back({x, y, z});

    return std::make_tuple(true, cur);
}

std::tuple<bool, const char*> parse_vertex_line(
    const char *src,
    const char *lim,
    tri_mesh &the_mesh,
    bool swapyz
)
{
    const char* cur = src;

    bool success;
    std::tie(success, cur) = parse_literal(cur, lim, "v");
    if(! success)
        return std::make_tuple(false, src);

    double x, y, z;
    std::tie(success, cur, x) = parse_double(cur, lim);
    if(! success)
        return std::make_tuple(false, src);
    std::tie(success, cur, y) = parse_double(cur, lim);
    if(! success)
        return std::make_tuple(false, src);
    std::tie(success, cur, z) = parse_double(cur, lim);
    if(! success)
        return std::make_tuple(false, src);

    std::tie(success, cur) = parse_newline(cur, lim);
    if(! success)
        return std::make_tuple(false, src);

    if(swapyz)
    {
        using std::swap;
        swap(y, z);
    }

    the_mesh.v.push_back({x, y, z});

    return std::make_tuple(true, cur);
}

/// Parse a "Wavefront obj"-style index triple.
///
/// Any unspecified indices will be returned as size_t's max value.
std::tuple<bool, const char*, std::array<size_t, 3>> parse_index_triple(
    const char *src,
    const char *lim
)
{
    const char *cur = src;

    std::array<size_t, 3> result = {
        std::numeric_limits<size_t>::max(),
        std::numeric_limits<size_t>::max(),
        std::numeric_limits<size_t>::max()
    };

    bool success;

    std::tie(success, cur, result[0]) = parse_size_t(cur, lim);
    if(! success)
        return std::make_tuple(false, src, result);

    // If don't get a slash next, we're done.
    std::tie(success, cur) = parse_literal(cur, lim, "/");
    if(! success)
        return std::make_tuple(true, cur, result);

    // This is allowed to fail, since the file doesn't have to specify a
    // texture-coordinate index, BUT it must then be immediately followed by
    // another slash.  If we do get a texture coordinate index, then we are done
    // if we aren't immediately followed by a slash.
    std::tie(success, cur, result[1]) = parse_size_t(cur, lim);
    if(! success)
    {
        std::tie(success, cur) = parse_literal(cur, lim, "/");
        if(! success)
            return std::make_tuple(false, src, result);
    }
    else
    {
        std::tie(success, cur) = parse_literal(cur, lim, "/");
        if(! success)
            return std::make_tuple(true, cur, result);
    }

    // Capture the normal index.  If we reached this point, a normal index is
    // required.
    std::tie(success, cur, result[2]) = parse_size_t(cur, lim);
    if(! success)
        return std::make_tuple(false, src, result);

    return std::make_tuple(true, cur, result);
}

std::tuple<bool, const char*> parse_face_line(
    const char *src,
    const char *lim,
    tri_mesh &the_mesh,
    bool swapyz
)
{
    const char *cur = src;

    bool success;
    std::tie(success, cur) = parse_literal(cur, lim, "f");
    if(! success)
        return std::make_tuple(false, src);

    std::array<std::array<size_t, 3>, 3> index_triples;

    std::tie(success, cur, index_triples[0]) = parse_index_triple(cur, lim);
    if(! success)
        return std::make_tuple(false, src);

    std::tie(success, cur, index_triples[1]) = parse_index_triple(cur, lim);
    if(! success)
        return std::make_tuple(false, src);

    std::tie(success, cur, index_triples[2]) = parse_index_triple(cur, lim);
    if(! success)
        return std::make_tuple(false, src);

    std::tie(success, cur) = parse_newline(cur, lim);
    if(! success)
        return std::make_tuple(false, src);

    tri_face_idx assembled = {
        {index_triples[0][0], index_triples[1][0], index_triples[2][0]},
        {index_triples[0][1], index_triples[1][1], index_triples[2][1]},
        {index_triples[0][2], index_triples[1][2], index_triples[2][2]}
    };

    if(swapyz)
    {
        using std::swap;

        swap(assembled.vi[1], assembled.vi[2]);
        swap(assembled.ni[1], assembled.ni[2]);
        swap(assembled.mi[1], assembled.mi[2]);
    }

    the_mesh.f.push_back(assembled);

    return std::make_tuple(true, cur);
}

std::tuple<bool, const char*> parse_group_line(const char *src, const char *lim)
{
    bool success;
    const char *cur = src;

    std::tie(success, cur) = parse_literal(cur, lim, "g");
    if(! success)
        return std::make_tuple(false, src);

    // Advance to something that could be a newline sequence.
    while(cur != lim && *cur != 0xa && *cur != 0xd)
        ++cur;

    std::tie(success, cur) = parse_newline(cur, lim);
    if(! success)
        return std::make_tuple(false, src);

    return std::make_tuple(true, cur);
}

std::tuple<bool, const char*> parse_mtllib_line(
    const char *src,
    const char *lim
)
{
    bool success;
    const char *cur = src;

    std::tie(success, cur) = parse_literal(cur, lim, "mtllib");
    if(! success)
        return std::make_tuple(false, src);

    // Advance to something that could be a newline sequence.
    while(cur != lim && *cur != 0xa && *cur != 0xd)
        ++cur;

    std::tie(success, cur) = parse_newline(cur, lim);
    if(! success)
        return std::make_tuple(false, src);

    return std::make_tuple(true, cur);
}

std::tuple<bool, const char*> parse_usemtl_line(
    const char *src,
    const char *lim
)
{
    bool success;
    const char *cur = src;

    std::tie(success, cur) = parse_literal(cur, lim, "usemtl");
    if(! success)
        return std::make_tuple(false, src);

    // Advance to something that could be a newline sequence.
    while(cur != lim && *cur != 0xa && *cur != 0xd)
        ++cur;

    std::tie(success, cur) = parse_newline(cur, lim);
    if(! success)
        return std::make_tuple(false, src);

    return std::make_tuple(true, cur);
}


std::tuple<int, size_t, tri_mesh> parse_obj(
    const char *src,
    const char *lim,
    bool swapyz
)
{
    tri_mesh the_mesh;
    size_t cur_line = 0;

    const char *cur = src;
    while(cur != lim)
    {
        ++cur_line;
        bool success;

        std::tie(success, cur) = parse_blank_line(cur, lim);
        if(success)
            continue;

        std::tie(success, cur) = parse_comment_line(cur, lim);
        if(success)
            continue;

        std::tie(success, cur) = parse_group_line(cur, lim);
        if(success)
            continue;

        std::tie(success, cur) = parse_mtllib_line(cur, lim);
        if(success)
            continue;

        std::tie(success, cur) = parse_usemtl_line(cur, lim);
        if(success)
            continue;

        std::tie(success, cur) = parse_vertex_line(cur, lim, the_mesh, swapyz);
        if(success)
            continue;

        std::tie(success, cur) = parse_texcoord_line(cur, lim, the_mesh);
        if(success)
            continue;

        std::tie(success, cur) = parse_normal_line(cur, lim, the_mesh, swapyz);
        if(success)
            continue;

        std::tie(success, cur) = parse_face_line(cur, lim, the_mesh, swapyz);
        if(success)
            continue;

        return std::make_tuple(OBJ_ERRC_PARSE_ERROR, cur_line, (tri_mesh()));
    }

    // Correct from 1-based indices to 0-based indices.
    for(tri_face_idx &idx : the_mesh.f)
    {
        for(size_t i = 0; i < 3; ++i)
        {
            --idx.vi[i];
            if(idx.ni[i] != std::numeric_limits<size_t>::max())
                --idx.ni[i];
            if(idx.mi[i] != std::numeric_limits<size_t>::max())
                --idx.mi[i];
        }
    }

    if(!tri_mesh_sanity_check(the_mesh))
        return std::make_tuple(OBJ_ERRC_INSANE, cur_line, (tri_mesh()));

    return std::make_tuple(OBJ_ERRC_NONE, cur_line, the_mesh);
}

std::tuple<int, size_t, tri_mesh> tri_mesh_load_obj(
    const std::string &filename,
    bool swapyz
)
{
    FILE *f = std::fopen(filename.c_str(), "rb");
    if(f == nullptr) {
        return std::make_tuple(OBJ_ERRC_FILE_NOT_OPENABLE, std::size_t(0), tri_mesh());
    }

    std::fseek(f, 0, SEEK_END);
    long fsize = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);

    std::vector<char> buf(fsize);
    std::fread(&buf[0], 1, fsize, f);
    if(std::ferror(f)) {
        std::fclose(f);
        return std::make_tuple(OBJ_ERRC_FILE_NOT_OPENABLE, std::size_t(0), tri_mesh());
    }
    std::fclose(f);

    int errc = OBJ_ERRC_NONE;
    size_t error_line = 0;
    tri_mesh the_mesh;

    std::tie(errc, error_line, the_mesh) = parse_obj(
        &buf[0],
        &buf[buf.size()],
        swapyz
    );
    return std::make_tuple(errc, error_line, the_mesh);
}

}

#endif
