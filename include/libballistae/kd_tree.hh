#ifndef LIBBALLISTAE_KD_TREE_HH
#define LIBBALLISTAE_KD_TREE_HH

#include <cassert>
#include <climits>
#include <cstddef>

#include <array>
#include <algorithm>
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
template<typename Field, typename StoredIt, size_t D>
struct aanode
{
    size_t depth;
    aabox<Field, D> bounds;
    StoredIt src;
    StoredIt lim;
    size_t link;
};

/// A balanced, bounded-volume
template<typename Field, size_t D, typename Stored>
struct kd_tree final
{
    std::vector<Stored> storage;
    std::vector<aanode<Field, decltype(storage.begin()), D>> nodes;

    size_t max_depth;

    template<typename StoredToAABox>
    kd_tree(
        std::vector<Stored> &&storage_in,
        StoredToAABox get_aabox
    );

    template<typename Selector, typename Computor>
    void query(Selector selector, Computor computor) const;
};

template<typename Field, size_t D, typename Stored>
template<typename StoredToAABox>
kd_tree<Field, D, Stored>::kd_tree(
    std::vector<Stored> &&storage_in,
    StoredToAABox get_aabox
)
    : storage(storage_in),
      max_depth(0)
{
    aabox<Field, D> max_box;
    if(storage.cbegin() != storage.cend())
    {
        max_box = std::accumulate(
            storage.cbegin(),
            storage.cend(),
            get_aabox(storage[0]),
            [&](auto a, auto b) {return min_containing(a, get_aabox(b));}
        );
    }

    aanode<Field, decltype(storage.begin()), D> root_node = {
        0,
        max_box,
        storage.begin(),
        storage.end(),
        std::numeric_limits<size_t>::max()
    };

    nodes.push_back(root_node);
}

template<typename Field, size_t D, typename Stored, typename StoredToAABox>
void kd_tree_refine_sah(
    kd_tree<Field, D, Stored> &tree,
    StoredToAABox get_aabox,
    Field split_cost,
    Field threshold
)
{
    using stored_it = typename std::vector<Stored>::iterator;

    size_t cur_node_idx = 0;
    while(cur_node_idx != tree.nodes.size())
    {
        auto &cur_node = tree.nodes[cur_node_idx];
        ++cur_node_idx;

        if(cur_node.depth + 1 > tree.max_depth)
            tree.max_depth = cur_node.depth + 1;

        bool should_cut;
        size_t cut_axis;
        Field cut;
        std::tie(should_cut, cut_axis, cut)
            = split_sah(cur_node, get_aabox, split_cost, threshold);

        // Terminate recursion if indicated.
        if(! should_cut)
            continue;

        // Sort on the chose axis.
        std::sort(
            cur_node.src,
            cur_node.lim,
            [&](auto a, auto b) {
                return aabox_axial_comparator(cut_axis, get_aabox(a), get_aabox(b));
            }
        );

        // Calculate the preceding, covering, and succeeding ranges for the
        // selected cut.
        std::array<stored_it, 4> split = {
            cur_node.src,
            cur_node.lim,
            cur_node.lim,
            cur_node.lim
        };

        split[1] = std::partition(
            split[0],
            split[3],
            [&](auto a) {
                return strictly_precedes(get_aabox(a).spans[cut_axis], cut);
            }
        );

        split[2] = std::partition(
            split[1],
            split[3],
            [&](auto a) {
                return contains(get_aabox(a).spans[cut_axis], cut);
            }
        );

        cur_node.link = tree.nodes.size();

        // Create and record the low child node.  It will hold all elements
        // preceding and overlapping the cut.

        aabox<Field, D> lo_bounds;
        if(split[2] - split[0] != 0)
        {
            lo_bounds = std::accumulate(
                split[0],
                split[2],
                get_aabox(*(split[0])),
                [&](auto a, auto b) {return min_containing(a, get_aabox(b));}
            );
        }

        aanode<Field, stored_it, D> lo_child = {
            cur_node.depth + 1,
            lo_bounds,
            split[0],
            split[2],
            std::numeric_limits<size_t>::max()
        };

        tree.nodes.push_back(lo_child);

        // Create and record the hi child node

        aabox<Field, D> hi_bounds;
        if(split[3] - split[2] != 0)
        {
            hi_bounds = std::accumulate(
                split[2],
                split[3],
                get_aabox(*(split[2])),
                [&](auto a, auto b) {return min_containing(a, get_aabox(b));}
            );
        }

        aanode<Field, stored_it, D> hi_child = {
            cur_node.depth + 1,
            hi_bounds,
            split[2],
            split[3],
            std::numeric_limits<size_t>::max()
        };

        tree.nodes.push_back(hi_child);
    }
}

template<typename Field, size_t D, typename StoredIt, typename StoredToAABox>
std::tuple<bool, size_t, Field> split_sah(
    aanode<Field, StoredIt, D> &parent,
    StoredToAABox get_aabox,
    Field split_cost,
    Field termination_threshold
)
{
    Field parent_objective
        = (parent.lim - parent.src) * surface_area(parent.bounds);

    size_t piv_axis;
    Field piv_cut;
    Field piv_objective = std::numeric_limits<Field>::infinity();

    for(size_t axis = 0; axis < D; ++axis)
    {
        // Sort on axis
        std::sort(
            parent.src,
            parent.lim,
            [&](auto a, auto b) {
                return aabox_axial_comparator(axis, get_aabox(a), get_aabox(b));
            }
        );

        for(auto el = parent.src; el != parent.lim; ++el)
        {
            auto box = get_aabox(*el);
            Field cut = box.spans[axis].hi;

            // Calculate the preceding, covering, and succeeding ranges for the
            // selected cut.
            std::array<StoredIt, 4> split = {
                parent.src,
                parent.lim,
                parent.lim,
                parent.lim
            };

            split[1] = std::partition(
                split[0],
                split[3],
                [&](auto a) {
                    return strictly_precedes(get_aabox(a).spans[axis], cut);
                }
            );

            split[2] = std::partition(
                split[1],
                split[3],
                [&](auto a) {
                    return contains(get_aabox(a).spans[axis], cut);
                }
            );

            Field lo_n = Field(split[2] - split[0]);
            Field hi_n = Field(split[3] - split[2]);

            aabox<Field, D> lo_box;
            if(split[2] - split[0] != 0)
            {
                lo_box = std::accumulate(
                    split[0],
                    split[2],
                    get_aabox(*(split[0])),
                    [&](auto a, auto b) {return min_containing(a, get_aabox(b));}
                );
            }

            aabox<Field, D> hi_box;
            if(split[3] - split[2] != 0)
            {
                hi_box = std::accumulate(
                    split[2],
                    split[3],
                    get_aabox(*(split[2])),
                    [&](auto a, auto b) {return min_containing(a, get_aabox(b));}
                );
            }

            Field lo_sa = surface_area(lo_box);
            Field hi_sa = surface_area(hi_box);

            Field objective = lo_sa * lo_n + hi_sa * hi_n;

            if(objective < piv_objective)
            {
                piv_axis = axis;
                piv_cut = cut;
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
    static thread_local std::vector<size_t> work_stack(max_depth);

    auto top = work_stack.begin();
    auto base = work_stack.begin();

    *top = 0;
    ++top;

    while(top != base)
    {
        --top;
        auto cur_idx = *top;
        auto cur_node = nodes[cur_idx];

        if(cur_node.src != cur_node.lim && selector(cur_node.bounds))
        {
            if(cur_node.link == std::numeric_limits<size_t>::max())
            {
                std::for_each(cur_node.src, cur_node.lim, computor);
            }
            else
            {
                *top = (cur_node.link + 1);
                ++top;
                *top = (cur_node.link + 0);
                ++top;
            }
        }
    }
}

}

#endif
