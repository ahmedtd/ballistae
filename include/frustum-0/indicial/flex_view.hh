
#ifndef FRUSTUM_0_FLEX_VIEW_HH
#define FRUSTUM_0_FLEX_VIEW_HH

#include "indicial_base.hh"

#include <cstddef>
#include <stdexcept>
#include <vector>

namespace frustum
{

// Forward declaration for the traits class.
template<class Derived>
struct flex_view;

// Traits class specialization.
template<class Derived>
struct indicial_traits<flex_view<Derived> >
{
    typedef typename indicial_traits<Derived>::entry_type entry_type;
};

namespace flex_view_detail
{

template<class, std::size_t, class...>
struct apply_offsets_to_param_pack;

template<
    class Derived,
    std::size_t AccessIndex,
    class... ParamPackTypes
    >
struct apply_offsets_to_param_pack<
    Derived,
    AccessIndex,
    std::tuple<ParamPackTypes...>
    >
{
    static std::size_t work(
        std::vector<std::size_t> offsets,
        ParamPackTypes... param_pack_vals
    )
    {
        return offsets[AccessIndex]
            + ct_ops::get_nth<AccessIndex, ParamPackTypes...>::work(
                param_pack_vals...
            );
    }
};
    

template<class...>
struct index_forward;

template<
    class Derived,
    std::size_t... AccessIndices,
    class... PassedIndexTypes
    >
struct index_forward<
    Derived,
    ct_ops::index_sequence<AccessIndices...>,
    PassedIndexTypes...
    >
{
    typedef typename indicial_traits<Derived>::entry_type entry_type;

    static entry_type& work(
        indicial_base<Derived> &data_owner,
        std::vector<std::size_t> &dim_offsets,
        PassedIndexTypes... passed_indices
    )
    {
        return data_owner(
            apply_offsets_to_param_pack<
                Derived,
                AccessIndices,
                std::tuple<PassedIndexTypes...>
            >::work(dim_offsets, passed_indices...)...
        );
    }
        
};

}

template<class Derived>
struct flex_view : indicial_base<flex_view<Derived> >
{
    typedef typename indicial_traits<Derived>::entry_type entry_type;

    // The actual owner of the data we're indexing into.
    indicial_base<Derived> &data_owner;
    
    // Indexing parameters.
    std::vector<std::size_t> dim_starts;
    std::vector<std::size_t> dim_sizes;
    
    // A flex_view cannot be created without a data owner, nor can it be
    // reseated among owners.
    flex_view(
        indicial_base<Derived> &data_owner,
        const std::vector<std::size_t> &dim_starts,
        const std::vector<std::size_t> &dim_sizes
    )
        : data_owner(data_owner),
          dim_starts(dim_starts),
          dim_sizes(dim_sizes)
    {
        if((data_owner.n_dims() != dim_starts.size())
           || (data_owner.n_dims() != dim_sizes.size()))
        {
            throw std::logic_error(
                "frustum: Cannot create flex_view into indicial of differing"
                " rank."
            );
        }

        for(std::size_t i = 0; i < data_owner.n_dims(); ++i)
        {
            if(dim_starts[i] >= data_owner.dim_size(i)
               || dim_starts[i] + dim_sizes[i] >= data_owner.dim_size(i)
            )
            {
                throw std::logic_error(
                    "frustum: flex_view extents must lie fully within the"
                    " bounds of the data_owner in all dimensions."
                );
            }
        }
    }

    std::size_t n_dims_impl() const
    {
        // Could also return index_extents.size(), but this has a better chance
        // of being known at compile time.
        return data_owner.n_dims();
    }

    std::size_t n_entries_impl() const
    {
        std::size_t prod_accum = 1;
        for(const std::size_t &cur : dim_sizes)
        {
            prod_accum *= cur;
        }
        return prod_accum;
    }

    std::size_t dim_size_impl(std::size_t i) const
    {
        return dim_sizes[i];
    }

    template<class... IndexTypes>
    entry_type& indexing_impl(IndexTypes... indices)
    {
        
        if(sizeof...(indices) == 1)
        {
            // This is a linear indexing into the view (will be known at compile
            // time).  Translate it into a linear indexing into the data source.
            throw std::logic_error(
                "frustum: Linear indexing into flex_view is not yet supported."
            );
        }
        else if(sizeof...(indices) == data_owner.n_dims())
        {
            // This is a ranked indexing into the view (might be known at
            // compile time, depending on the type of data_owner).  Translate
            // the ranked indexing into a linear indexing into the data source.

            typedef typename ct_ops::make_index_sequence<
                sizeof...(indices)
                >::type
                AccessIndices;

            return flex_view_detail::index_forward<
                Derived,
                AccessIndices,
                IndexTypes...>::
                work(
                    data_owner,
                    dim_starts,
                    indices...
                );
        }
        else
        {
            // The user gave the wrong number of indices (might be known at
            // compile time, depending on the type of data_owner).
            throw std::logic_error(
                "frustum: Incorrect number of indices given to flex_view."
            );
        }
    }
};

template<class Derived>
flex_view<Derived> make_flex_view(
    indicial_base<Derived> &data_owner,
    const std::vector<std::size_t> &dim_starts,
    const std::vector<std::size_t> &dim_sizes
)
{
    return flex_view<Derived>(data_owner, dim_starts, dim_sizes);
}

}

#endif
