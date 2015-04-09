#ifndef LIBBALLISTAE_KD_TREE_HH
#define LIBBALLISTAE_KD_TREE_HH

#include <cassert>
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
/// Each aacut at a given depth owns a portion of the storage array, which is
/// pre-divided into three different ranges at tree construction time.
///
///   * The first range, [partitions[0], partitions[1]), consists of elements
///     whose aaboxes strictly precede cut_plane along axis.
///
///   * The second range, [partitions[1], partitions[2]), consists of elements
///     whose aaboxes contain cut_plane, along axis.
///
///   * The third range, [partitions[2], partitions[3]), consists of elements
///     whose aaboxes strictly succeed cut_plane along axis.
///
/// If an aacut is not a leaf, then the low child is the aacut immediately
/// following it in the kd_tree cut vector.  The high child can be found at
/// kd_tree.cuts[link] (in a properly-initialized kd_tree).
template<typename Field, typename StoredIt, size_t D>
struct aacut
{
    aabox<Field, D> bounds;
    std::array<StoredIt, 4> partitions;
    size_t link;
};

/// A balanced, bounded-volume
template<typename Field, size_t D, typename Stored>
struct kd_tree final
{
    std::vector<Stored> storage;
    std::vector<aacut<Field, decltype(storage.begin()), D>> cuts;
    size_t bucket_cap;

    template<typename StoredIt,typename StoredToAABox>
    kd_tree(
        StoredIt src,
        StoredIt lim,
        size_t bucket_cap,
        StoredToAABox get_aabox
    );


    template<typename Selector, typename Computor>
    void query(Selector selector, Computor computor) const;
};

template<typename Field, size_t D, typename Stored>
template<typename StoredIt, typename StoredToAABox>
kd_tree<Field, D, Stored>::kd_tree(
    StoredIt src_in,
    StoredIt lim_in,
    size_t bucket_cap_in,
    StoredToAABox get_aabox
)
    : storage(std::distance(src_in, lim_in)),
      cuts(),
      bucket_cap(bucket_cap_in)
{
    std::move(src_in, lim_in, storage.begin());

    if(storage.cbegin() == storage.cend())
    {
        aacut<Field, decltype(storage.begin()), D> top_cut;
        for(size_t i = 0; i < D; ++i)
            top_cut.bounds.spans[i] = {0.0, 0.0};
        top_cut.partitions = {storage.end(), storage.end(), storage.end(), storage.end()};
        top_cut.link = std::numeric_limits<size_t>::max();
        cuts.push_back(top_cut);
        return;
    }

    aabox<Field, D> max_box = std::accumulate(
        storage.cbegin(),
        storage.cend(),
        get_aabox(storage[0]),
        [&](auto a, auto b) {return min_containing(a, get_aabox(b));}
    );

    // During tree construction, unfinalized aacuts have their link set to the
    // parent node.  As nodes are finalized, they patch their parent's link to
    // point to themselves, then set their own link to size_t max as a sentinel.
    // The order in which nodes are processed ensures that all low siblings are
    // processed before all high siblings, so the parent's link will be set to
    // point to their high child.

    std::vector<aacut<Field, decltype(storage.begin()), D>> work_stack;
    work_stack.push_back({max_box, {storage.begin(), storage.end(), storage.end(), storage.end()}, std::numeric_limits<size_t>::max()});

    while(! work_stack.empty())
    {
        auto cur_cut = work_stack.back();
        work_stack.pop_back();

        // Terminate recursion when the current bucket is small enough.
        if(static_cast<size_t>(cur_cut.partitions[3] - cur_cut.partitions[0])
           < bucket_cap)
        {
            if(cur_cut.link != std::numeric_limits<size_t>::max())
                cuts[cur_cut.link].link = cuts.size();
            cur_cut.link = std::numeric_limits<size_t>::max();
            cuts.push_back(cur_cut);
            continue;
        }

        // Pick an axis on which to divide the current cut.
        size_t div_axis = 0;
        size_t div_measure = 0.0;
        for(size_t i = 0; i < D; ++i)
        {
            if(measure(cur_cut.bounds.spans[i]) > div_measure)
            {
                div_axis = i;
                div_measure = measure(cur_cut.bounds.spans[i]);
            }
        }

        // Find the median element along the chosen axis.
        std::sort(
            cur_cut.partitions[0],
            cur_cut.partitions[3],
            [&](auto a, auto b){
                return aabox_axial_comparator(div_axis, get_aabox(a), get_aabox(b));
            }
        );
        auto median = cur_cut.partitions[0] + (cur_cut.partitions[3] - cur_cut.partitions[0]) / 2;

        // Get a single cut value.
        Field cut_val = get_aabox(*median).spans[div_axis].hi;

        // Calculate the preceding, covering, and succeeding ranges for the
        // selected cut.

        cur_cut.partitions[1] = std::partition(
            cur_cut.partitions[0],
            cur_cut.partitions[3],
            [&](auto a) {
                return strictly_precedes(get_aabox(a).spans[div_axis], cut_val);
            }
        );

        cur_cut.partitions[2] = std::partition(
            cur_cut.partitions[1],
            cur_cut.partitions[3],
            [&](auto a) {
                return contains(get_aabox(a).spans[div_axis], cut_val);
            }
        );

        // Queue up the current cut's two children for inspection.  Low child
        // will be processed next.
        auto new_aaboxes = cut(cur_cut.bounds, div_axis, cut_val);

        auto low_child = cur_cut;
        low_child.bounds = new_aaboxes[0];
        low_child.link = cuts.size();
        low_child.partitions[0] = cur_cut.partitions[0];
        low_child.partitions[3] = cur_cut.partitions[1];

        auto high_child = cur_cut;
        high_child.bounds = new_aaboxes[1];
        high_child.link = cuts.size();
        high_child.partitions[0] = cur_cut.partitions[2];
        high_child.partitions[3] = cur_cut.partitions[3];

        work_stack.push_back(high_child);
        work_stack.push_back(low_child);

        // Record the current cut for use in lookups, patching the parent link.
        if(cur_cut.link != std::numeric_limits<size_t>::max())
            cuts[cur_cut.link].link = cuts.size();
        cuts.push_back(cur_cut);
    }
}

template<typename Field, size_t D, typename Stored>
template<typename Selector, typename Computor>
void kd_tree<Field, D, Stored>::query(Selector selector, Computor computor)
    const
{
    static thread_local std::vector<size_t> work_stack;
    work_stack.clear();
    work_stack.push_back(0);
    while(! work_stack.empty())
    {
        auto cur_idx = work_stack.back();
        auto cur_cut = cuts[cur_idx];
        work_stack.pop_back();

        if(selector(cur_cut.bounds))
        {
            if(cur_cut.link == std::numeric_limits<size_t>::max())
            {
                std::for_each(
                    cur_cut.partitions[0],
                    cur_cut.partitions[3],
                    computor
                );
            }
            else
            {
                // Push right child.
                assert(cur_cut.link < cuts.size());
                work_stack.push_back(cur_cut.link);

                // Push left child (will be processed right now).
                assert(cur_idx + 1 < cuts.size());
                work_stack.push_back(cur_idx + 1);

                // Process elements that fall directly on the cut.
                std::for_each(
                    cur_cut.partitions[1],
                    cur_cut.partitions[2],
                    computor
                );
            }
        }
    }
}

}

#endif
