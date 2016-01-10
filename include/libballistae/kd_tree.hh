#ifndef LIBBALLISTAE_KD_TREE_HH
#define LIBBALLISTAE_KD_TREE_HH

#include <cassert>
#include <climits>
#include <cstddef>

#include <algorithm>
#include <array>
#include <memory>
#include <vector>

#include <libballistae/aabox.hh>
#include <libballistae/kd_tree.hh>
#include <libballistae/span.hh>

namespace ballistae
{

///
///
/// The LINK field is an index into the kd tree's NODES field, and points to a
/// the source of a two-node block holding the children of this node.
template<typename Field, size_t D, typename Stored>
struct aanode
{
    aabox<Field, D> bounds;

    typename std::vector<Stored>::iterator elements_src;
    typename std::vector<Stored>::iterator elements_lim;

    std::unique_ptr<aanode<Field, D, Stored>> lo_child;
    std::unique_ptr<aanode<Field, D, Stored>> hi_child;
};

/// A balanced, bounded-volume
template<typename Field, size_t D, typename Stored>
struct kd_tree final
{
    std::vector<Stored> infinite_elements;
    std::vector<Stored> finite_elements;
    std::unique_ptr<aanode<Field, D, Stored>> root;

    kd_tree() = default;

    kd_tree(const kd_tree<Field, D, Stored> &other) = delete;
    kd_tree(kd_tree<Field, D, Stored> &&other) = default;

    template<typename StoredToAABox>
    kd_tree(
        std::vector<Stored> &&storage_in,
        StoredToAABox get_aabox
    );

    template<typename Selector, typename Computor>
    void query(Selector selector, Computor computor) const;

    kd_tree<Field, D, Stored>& operator=(const kd_tree<Field, D, Stored> &other) = delete;
    kd_tree<Field, D, Stored>& operator=(kd_tree<Field, D, Stored> &&other) = default;
};

template<typename Field, size_t D, typename Stored>
template<typename StoredToAABox>
kd_tree<Field, D, Stored>::kd_tree(
    std::vector<Stored> &&storage,
    StoredToAABox get_aabox
)
{
    using std::begin;
    using std::end;

    using std::make_move_iterator;

    // Split the provided elements into finite and infinite elements.
    auto finite_lim = std::partition(
        begin(storage),
        end(storage),
        [&](auto &x){return isfinite(get_aabox(x));}
    );
    finite_elements = std::vector<Stored>(
        make_move_iterator(begin(storage)),
        make_move_iterator(finite_lim)
    );
    infinite_elements = std::vector<Stored>(
        make_move_iterator(finite_lim),
        make_move_iterator(end(storage))
    );

    aabox<Field, D> max_box = std::accumulate(
        std::begin(finite_elements),
        std::end(finite_elements),
        aabox<Field, D>::accum_zero(),
        [&](auto a, auto b) {return min_containing(a, get_aabox(b));}
    );

    // Root node encompasses all finite boxes.
    root = std::make_unique<aanode<Field, D, Stored>>();
    *root = {
        max_box,
        begin(finite_elements),
        end(finite_elements),
        nullptr,
        nullptr
    };
}

template<typename Field, size_t D, typename Stored, typename StoredToAABox>
void kd_tree_refine_sah(
    aanode<Field, D, Stored> *cur,
    StoredToAABox get_aabox,
    Field split_cost,
    Field threshold
)
{
    using std::begin;
    using std::end;

    using std::make_move_iterator;

    using std::distance;

    if(distance(cur->elements_src, cur->elements_lim) < 2)
        return;

    bool should_cut;
    size_t cut_axis;
    Field cut;
    std::tie(should_cut, cut_axis, cut) = split_sah(
        *(cur),
        get_aabox,
        split_cost,
        threshold
    );

    // Terminate recursion if indicated.
    if(! should_cut)
        return;

    // Place elements that strictly precede the cut into the low child of the
    // current node.
    auto precede_src = cur->elements_src;
    auto precede_lim = std::partition(
        cur->elements_src,
        cur->elements_lim,
        [&](auto &a) {
            return strictly_precedes(get_aabox(a).spans[cut_axis], cut);
        }
    );

    if(distance(precede_src, precede_lim) != 0)
    {
        aabox<Field, D> lo_bounds = std::accumulate(
            precede_src,
            precede_lim,
            aabox<Field, D>::accum_zero(),
            [&](auto a, auto b) {return min_containing(a, get_aabox(b));}
        );

        cur->lo_child = std::make_unique<aanode<Field, D, Stored>>();
        *(cur->lo_child) = {
            lo_bounds,
            precede_src,
            precede_lim,
            nullptr,
            nullptr
        };

        kd_tree_refine_sah(cur->lo_child.get(), get_aabox, split_cost, threshold);
    }

    // All remaining elements overlap or strictly succeed the cut, and are
    // placed in the high child.

    auto succeed_src = precede_lim;
    auto succeed_lim = cur->elements_lim;

    if(distance(succeed_src, succeed_lim) != 0)
    {
        aabox<Field, D> hi_bounds = std::accumulate(
            succeed_src,
            succeed_lim,
            aabox<Field, D>::accum_zero(),
            [&](auto a, auto b) {return min_containing(a, get_aabox(b));}
        );

        cur->hi_child = std::make_unique<aanode<Field, D, Stored>>();
        *(cur->hi_child) = {
            hi_bounds,
            succeed_src,
            succeed_lim,
            nullptr,
            nullptr
        };

        kd_tree_refine_sah(cur->hi_child.get(), get_aabox, split_cost, threshold);
    }
}

template<typename Field, size_t D, typename Stored, typename StoredToAABox>
std::tuple<bool, size_t, Field> split_sah(
    aanode<Field, D, Stored> &cur,
    StoredToAABox get_aabox,
    Field split_cost,
    Field termination_threshold
)
{
    using std::begin;
    using std::end;

    using std::distance;

    Field parent_objective
        = distance(cur.elements_src, cur.elements_lim) * surface_area(cur.bounds);

    size_t piv_axis = 0;
    Field piv_cut;
    Field piv_objective = std::numeric_limits<Field>::infinity();

    for(size_t axis = 0; axis < D; ++axis)
    {
        // Sort on axis
        std::sort(
            cur.elements_src,
            cur.elements_lim,
            [&](auto a, auto b) {
                return aabox_axial_comparator(axis, get_aabox(a), get_aabox(b));
            }
        );

        //auto median_it = cur.elements_src + distance(cur.elements_src, cur.elements_lim) / 2;

        for(auto it = cur.elements_src; it != cur.elements_lim; ++it)
        {
            auto precede_src = cur.elements_src;
            auto precede_lim = std::find_if_not(
                cur.elements_src,
                cur.elements_lim,
                [&](auto &e) {
                    return strictly_precedes(get_aabox(e).spans[axis], get_aabox(*it).spans[axis].hi);
                }
            );

            auto succeed_src = precede_lim;
            auto succeed_lim = cur.elements_lim;

            aabox<Field, D> lo_box = std::accumulate(
                precede_src,
                precede_lim,
                aabox<Field, D>::accum_zero(),
                [&](auto a, auto b) {return min_containing(a, get_aabox(b));}
            );

            aabox<Field, D> hi_box = std::accumulate(
                succeed_src,
                succeed_lim,
                aabox<Field, D>::accum_zero(),
                [&](auto a, auto b) {return min_containing(a, get_aabox(b));}
            );

            Field objective = Field(0);
            if(distance(precede_src, precede_lim) != 0)
                objective += surface_area(lo_box) * distance(precede_src, precede_lim);
            if(distance(succeed_src, succeed_lim) != 0)
                objective += surface_area(hi_box) * distance(succeed_src, succeed_lim);

            if(objective < piv_objective)
            {
                piv_axis = axis;
                piv_cut = get_aabox(*it).spans[axis].hi;
                piv_objective = objective;
            }
        }
    }

    // Now we have computed the optimal split.  We need to check if it is a good
    // improvement over simply not splitting.
    if(piv_objective + split_cost > termination_threshold * parent_objective)
    {
        return std::make_tuple(false, 0, Field(0));
    }
    else
    {
        return std::make_tuple(true, piv_axis, piv_cut);
    }
}

template<typename Field, size_t D, typename Stored>
template<typename Selector, typename Computor>
void kd_tree<Field, D, Stored>::query(
    Selector selector,
    Computor computor
)
    const
{
    using std::begin;
    using std::end;

    // Always check all infinite elements.
    std::for_each(begin(infinite_elements), end(infinite_elements), computor);

    // We recurse to a fixed depth.
    std::array<aanode<Field, D, Stored>*, 2048> work_stack;

    auto top = begin(work_stack);
    auto base = begin(work_stack);

    *top = root.get();
    ++top;

    while(top != base)
    {
        --top;
        auto p_cur = *top;

        if(selector(p_cur->bounds))
        {
            if(p_cur->lo_child == nullptr && p_cur->hi_child == nullptr)
            {
                std::for_each(
                    p_cur->elements_src,
                    p_cur->elements_lim,
                    computor
                );
            }
            else if(static_cast<size_t>(top - base) < work_stack.size() - 2)
            {
                if(p_cur->lo_child != nullptr)
                {
                    *top = p_cur->lo_child.get();
                    ++top;
                }
                if(p_cur->hi_child != nullptr)
                {
                    *top = p_cur->hi_child.get();
                    ++top;
                }
            }
        }
    }
}

}

#endif
