
#ifndef FRUSTUM_MATRIX_FLEX_HH
#define FRUSTUM_MATRIX_FLEX_HH

#include "indicial_base.hh"

#include <algorithm>
#include <initializer_list>
#include <stdexcept>
#include <vector>

#include "../utility/compile_time_ops.hh"
#include "assign_helper.hh"

namespace frustum
{

// Forward declaration of flex.
template<class Entry>
struct flex;

// 
// Utility namespace for flex.
//
namespace flex_detail
{

//
// Assign nested initializer lists into a range. (Base case)
//
template<class ListEntry>
struct initializer
{
    template<class ContainerEntry>
    static void work(
        std::vector<ContainerEntry> &container,
        std::vector<std::size_t> &dimensions,
        const std::initializer_list<ListEntry> source
    )
    {
        initializer<std::initializer_list<ListEntry> >::work_helper(
            container,
            dimensions,
            source,
            true,
            0
        );
    }

    template<class ContainerEntry>
    static std::size_t work_helper(
        std::vector<ContainerEntry> &container,
        std::vector<std::size_t> &dimensions,
        const std::initializer_list<ListEntry> source,
        bool first_at_level,
        std::size_t expected_size
    )
    {
        if(first_at_level)
        {
            dimensions.push_back(source.size());
        }
        else if(source.size() != expected_size)
        {
            throw std::logic_error(
                "Frustum:: size mismatch in shaped initialization."
            );
        }

        container.insert(end(container), begin(source), end(source));

        return source.size();
    }
};

// Recursive case.
template<class ListEntry>
struct initializer<std::initializer_list<ListEntry> >
{
    // This function just dispatches to work_helper at the same recursion level.
    template<class ContainerEntry>
    static void work(
        std::vector<ContainerEntry> &container,
        std::vector<std::size_t> &dimensions,
        const std::initializer_list<std::initializer_list<ListEntry> > source
    )
    {
        initializer<std::initializer_list<ListEntry> >::work_helper(
            container,
            dimensions,
            source,
            true,
            0
        );
    }

    // This functions unwraps a layer of intializer lists.
    template<class ContainerEntry>
    static std::size_t work_helper(
        std::vector<ContainerEntry> &container,
        std::vector<std::size_t> &dimensions,
        const std::initializer_list<std::initializer_list<ListEntry> > source,
        bool first_at_level,
        std::size_t expected_size
    )
    {
        // The size of the initializer list at this level is the dimension for
        // this index.
        if(first_at_level)
        {
            dimensions.push_back(source.size());
        }
        else if(source.size() != expected_size)
        {
            throw std::logic_error(
                "Frustum:: size mismatch in shaped initialization."
            );
        }

        bool first = true;
        std::size_t level_size = 0;
        for(const auto cur_list : source)
        {
            level_size = initializer<ListEntry>::work_helper(
                container,
                dimensions,
                cur_list,
                first,
                level_size
            );
            
            first = false;
        }

        return source.size();
    }
};

struct result_struct
{
    std::size_t offset;
    std::size_t size_below;
};

//
// Turn a variable-length list of integer arguments into an index into the
// flex-sized indicial.
//
template<class IndicesHeadType, class... IndicesTailTypes>
struct address
{
    template<class DimIt>
    static std::size_t work(
        DimIt dims_begin,
        DimIt dims_end,
        IndicesHeadType indices_head,
        IndicesTailTypes... indices_tail
    )
    {
        result_struct results = address<IndicesHeadType, IndicesTailTypes...>
            ::work_helper(
            dims_begin,
            dims_end,
            indices_head,
            indices_tail...
        );
        return results.offset;
    }

    template<class DimIt>
    static result_struct work_helper(
        DimIt dims_begin,
        DimIt dims_end,
        IndicesHeadType indices_head,
        IndicesTailTypes... indices_tail
            )
    {
        result_struct results = address<IndicesTailTypes...>::work_helper(
            dims_begin + 1,
            dims_end,
            indices_tail...
        );
        results.offset += indices_head * results.size_below;
        results.size_below *= (*dims_begin);
        return results;
    }
};

template<class IndicesFinalType>
struct address<IndicesFinalType>
{
    template<class DimIt>
    static std::size_t work(
        DimIt dims_begin,
        DimIt dims_end,
        IndicesFinalType indices_final
    )
    {
        result_struct results = address<IndicesFinalType>::work_helper(
            dims_begin,
            dims_end,
            indices_final
        );
        return results.offset;
    }

    template<class DimIt>
    static result_struct work_helper(
        DimIt dims_begin,
        DimIt dims_end,
        IndicesFinalType indices_final
    )
    {
        // TODO: This cast is a hack.
        result_struct results = {(std::size_t)indices_final, *dims_begin};
        return results;
    }
};

// End namespace flex_detail.

}

template<class Entry>
struct indicial_traits<flex<Entry> >
{
    typedef Entry entry_type;
};

template<class Entry>
struct flex : public indicial_base<flex<Entry> >
{    
public:
    std::vector<std::size_t> dimensions;
    std::vector<Entry> container;

public:
    /// Default constructor (no dimensions, no entries).
    flex()
    {
    }

    /// Copy construction of same type.  Needs to be here because otherwise the
    /// compiler implicitly deletes it.  In theory, the implicit type conversion
    /// constructor should cover this case.
    flex(const flex<Entry> &other)
        : dimensions(other.dimensions),
          container(other.container)
    {
    }

    /// Copy construction (with implicit type conversion).
    template<class OtherEntry>
    flex(const flex<OtherEntry> &other)
        : dimensions(other.dimensions),
          container(other.container.size())
    {
        std::copy(
            begin(other.container),
            end(other.container),
            begin(container)
        );
    }

    /// Move construction (must be same type).
    flex(flex<Entry> &&other)
        : dimensions(move(other.dimensions)),
          container(move(other.container))
    {
    }

    /// Sized construction.
    ///
    /// Filled with default-initialized elements.  For basic numeric types, this
    /// means no initialization at all.
    template<class... DimTypes>
    flex(DimTypes... dimvalues)
        : dimensions(sizeof...(DimTypes)),
          container(ct_ops::argpack_mul<DimTypes...>::work(dimvalues...))
    {
        ct_ops::argpack_to_range<DimTypes...>::work(
            begin(dimensions),
            dimvalues...
        );
    }

    // Need to add single-value initialization in a way that doesn't conflict
    // with the plain sized construction.  Think about the case where you're
    // trying to fill a flex of std::size_t.

    /// Shaped (nested) initializer list.
    ///
    /// Will throw if the sizes of the initializer list levels are inconsistent.
    template<class MaybeEntryOrList>
    flex(std::initializer_list<MaybeEntryOrList> init)
    {
        flex_detail::initializer<MaybeEntryOrList>
            ::work(container, dimensions, init);
    }

    std::size_t n_dims_impl () const
    {
        return dimensions.size();
    }
    
    std::size_t dim_size_impl(const std::size_t which_dim) const
    {
        return dimensions[which_dim];
    }

    std::size_t n_entries_impl() const
    {
        return container.size();
    }

    // Both linear and normal access handled by the same machinery.
    template<class... IndexTypes>
    Entry& indexing_impl(IndexTypes... indices)
    {
        return container[
            flex_detail::address<IndexTypes...>::work(
                begin(dimensions),
                end(dimensions),
                indices...
            )
        ];
    }

    template<class... IndexTypes>
    const Entry& indexing_impl(IndexTypes... indices) const
    {
        return container[
            flex_detail::address<IndexTypes...>::work(
                begin(dimensions),
                end(dimensions),
                indices...
            )
        ];
    }
    
    // Compiler will delete otherwise, since we define a move constructor.
    flex<Entry>& operator=(const flex<Entry>& source)
    {
        return crtp::cast<flex<Entry> >(
            assign_detail::assign_helper<
            indicial_base<flex<Entry>>,
            indicial_base<flex<Entry>>
            >
            ::assign(source, *this)
        );
    }
    
    // // Basic assignment.  other is a universal reference, not necessarily an
    // // rvalue-reference.  If it turns out to be an rvalue reference at bind
    // // time, the appropriately specialized helper class will be used.  I think
    // // it can also be const.
    // template<class OtherEntry>
    // flex<Entry>& operator=(flex<OtherEntry> &&other)
    // {
    //     return assign_detail::assign_helper<decltype(other), decltype(*this)>(
    //         other,
    //         *this
    //     );
    // }

    // // Need to add an overload taking a const argument because declaring a move
    // // constructor implicitly deletes it.
    // flex<Entry>& operator=(const flex<Entry> &source)
    // {
    //     return assign_detail::assign_helper<decltype(source), decltype(*this)>(
    //         *(static_cast<const indicial_base<flex<Entry> >*>(&source)),
    //         *(static_cast<indicial_base<flex<Entry> >*>(this))
    //     );
    // }

    // // Add-addition.  Same deal with the universal reference.
    // template<class OtherEntry>
    // flex<Entry>& operator+=(flex<OtherEntry> &&other)
    // {
    //     return assign_detail::add_assign_helper<
    //         decltype(other),
    //         decltype(*this)
    //     >(
    //         other,
    //         *this
    //     );
    // }

    // // Need to add an overload taking a const argument because declaring a move
    // // constructor implicitly deletes it.
    // flex<Entry>& operator+=(const flex<Entry> &source)
    // {
    //     return assign_detail::add_assign_helper<
    //         decltype(source),
    //         decltype(*this)
    //     >(
    //         source,
    //         *this
    //     );
    // }

    // // Subtract-addition.  Same deal with with universal reference.
    // template<class OtherEntry>
    // flex<Entry>& operator-=(flex<OtherEntry> &&other)
    // {
    //     return assign_detail::subtract_assign_helper<
    //         decltype(other),
    //         decltype(this)
    //     >(
    //         other,
    //         *this
    //     );
    // }

    // // Need to add an overload taking a const argument because declaring a move
    // // constructor implicitly deletes it.
    // flex<Entry>& operator-=(const flex<Entry> &source)
    // {
    //     return assign_detail::subtract_assign_helper<
    //         decltype(source),
    //         decltype(*this)
    //     >(
    //         source,
    //         *this
    //     );
    // }

    template<class OtherType>
    auto assign_impl(OtherType &&other)
        -> decltype(
            assign_detail::assign_helper<decltype(other), decltype(*this)>
            ::assign(other, *this)
        )
    {
        return
            assign_detail::assign_helper<decltype(other), decltype(*this)>
            ::assign(other, *this);
    }

    template<class OtherType>
    auto add_assign_impl(OtherType &&other)
        -> decltype(
            assign_detail::add_assign_helper<decltype(other), decltype(*this)>
            ::add_assign(other, *this)
        )
    {
        return
            assign_detail::add_assign_helper<decltype(other), decltype(*this)>
            ::add_assign(other, *this);
    }

    template<class OtherType>
    auto subtract_assign_impl(OtherType &&other)
        -> decltype(
            assign_detail::subtract_assign_helper<decltype(other), decltype(*this)>
            ::add_assign(other, *this)
        )
    {
        return
            assign_detail::subtract_assign_helper<decltype(other), decltype(*this)>
            ::add_assign(other, *this);
    }
};

}

#endif
