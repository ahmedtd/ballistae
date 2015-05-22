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
template<typename Field, typename Stored, size_t D>
struct aanode
{
    size_t depth;
    aabox<Field, D> bounds;
    typename std::vector<Stored>::iterator src;
    typename std::vector<Stored>::iterator lim;
    size_t link;
};

/// A balanced, bounded-volume
template<typename Field, size_t D, typename Stored>
struct kd_tree final
{
    std::vector<Stored> storage;
    std::vector<aanode<Field, Stored, D>> nodes;

    /// The storage vector is partitioned into elements with finite aaboxes in
    /// the range [finite_src, finite_lim), and infinite aaboxes in the range
    /// [finite_lim, infinite_lim).
    typename std::vector<Stored>::iterator finite_src;
    typename std::vector<Stored>::iterator finite_lim;
    typename std::vector<Stored>::iterator infinite_lim;

    size_t max_depth;

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
    std::vector<Stored> &&storage_in,
    StoredToAABox get_aabox
)
    : storage(storage_in),
      finite_src(storage.begin()),
      finite_lim(storage.end()),
      infinite_lim(storage.end()),
      max_depth(0)
{
    // Pull finite boxes forward, push infinite ones back.
    finite_lim = std::partition(
        finite_src,
        infinite_lim,
        [&](auto &x){return isfinite(get_aabox(x));}
    );

    aabox<Field, D> max_box;
    if(finite_src != finite_lim)
    {
        max_box = std::accumulate(
            finite_src,
            finite_lim,
            get_aabox(storage[0]),
            [&](auto a, auto b) {return min_containing(a, get_aabox(b));}
        );
    }

    // Root node encompasses all finite boxes.
    aanode<Field, Stored, D> root_node = {
        0,
        max_box,
        finite_src,
        finite_lim,
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
    size_t cur_node_idx = 0;
    while(cur_node_idx != tree.nodes.size())
    {
        size_t cur_depth = tree.nodes[cur_node_idx].depth;

        if(cur_depth + 1 > tree.max_depth)
            tree.max_depth = cur_depth + 1;

        bool should_cut;
        size_t cut_axis;
        Field cut;
        std::tie(should_cut, cut_axis, cut)
            = split_sah(tree.nodes[cur_node_idx], get_aabox, split_cost, threshold);

        // Terminate recursion if indicated.
        if(! should_cut)
        {
            ++cur_node_idx;
            continue;
        }

        // Sort on the chosen axis.
        std::sort(
            tree.nodes[cur_node_idx].src,
            tree.nodes[cur_node_idx].lim,
            [&](auto a, auto b) {
                return aabox_axial_comparator(cut_axis, get_aabox(a), get_aabox(b));
            }
        );

        // Calculate the preceding, covering, and succeeding ranges for the
        // selected cut.
        std::array<typename std::vector<Stored>::iterator, 4> split = {
            tree.nodes[cur_node_idx].src,
            tree.nodes[cur_node_idx].lim,
            tree.nodes[cur_node_idx].lim,
            tree.nodes[cur_node_idx].lim
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

        tree.nodes[cur_node_idx].link = tree.nodes.size();

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

        aanode<Field, Stored, D> lo_child = {
            cur_depth + 1,
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

        aanode<Field, Stored, D> hi_child = {
            cur_depth + 1,
            hi_bounds,
            split[2],
            split[3],
            std::numeric_limits<size_t>::max()
        };

        tree.nodes.push_back(hi_child);

        ++cur_node_idx;
    }
}

template<typename Field, size_t D, typename Stored, typename StoredToAABox>
std::tuple<bool, size_t, Field> split_sah(
    aanode<Field, Stored, D> &parent,
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
            Field cut = box.spans[axis].hi();

            // Calculate the preceding, covering, and succeeding ranges for the
            // selected cut.
            std::array<typename std::vector<Stored>::iterator, 4> split = {
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
    // We always have to query all non-finite elements.
    std::for_each(finite_lim, infinite_lim, computor);

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
