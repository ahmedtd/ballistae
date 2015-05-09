#ifndef TRI_MESH_HH
#define TRI_MESH_HH

#include <array>
#include <vector>

#include <libballistae/aabox.hh>
#include <libballistae/contact.hh>
#include <libballistae/kd_tree.hh>
#include <libballistae/ray.hh>
#include <libballistae/span.hh>
#include <libballistae/vector.hh>

struct tri_face_verts
{
    ballistae::fixvec<double, 3> v0;
    ballistae::fixvec<double, 3> v1;
    ballistae::fixvec<double, 3> v2;
};

struct tri_face_normals
{
    ballistae::fixvec<double, 3> v0;
    ballistae::fixvec<double, 3> v1;
    ballistae::fixvec<double, 3> v2;
};

struct tri_face_texcoords
{
    ballistae::fixvec<double, 2> v0;
    ballistae::fixvec<double, 2> v1;
    ballistae::fixvec<double, 2> v2;
};

/// 3-element indexer.  Used to define face vertices, normals, and material
/// coordinates.
struct tri_face_idx
{
    std::array<size_t, 3> vi;
    std::array<size_t, 3> mi;
    std::array<size_t, 3> ni;
};

struct tri_mesh
{
    std::vector<ballistae::fixvec<double, 3>> v;
    std::vector<ballistae::fixvec<double, 3>> n;
    std::vector<ballistae::fixvec<double, 2>> m;

    std::vector<tri_face_idx> f;
};

tri_face_verts load_face_v(const tri_mesh &mesh, const tri_face_idx &idx)
    __attribute__((visibility("hidden")));
tri_face_normals load_face_n(const tri_mesh &mesh, const tri_face_idx &idx)
    __attribute__((visibility("hidden")));
tri_face_texcoords load_face_m(const tri_mesh &mesh, const tri_face_idx &idx)
    __attribute__((visibility("hidden")));

struct tri_face_crunched
{
    ballistae::fixvec<double, 3> v0;
    ballistae::fixvec<double, 3> u;
    ballistae::fixvec<double, 3> v;
    ballistae::fixvec<double, 3> n;

    double uu;
    double vv;
    double uv;
    double recip_denom;
};

ballistae::aabox<double, 3> get_aabox(const tri_face_crunched &f)
    __attribute__((visibility("hidden")));

ballistae::kd_tree<double, 3, tri_face_crunched> crunch(const tri_mesh &m)
    __attribute__((visibility("hidden")));

constexpr int CONTACT_INTO = (1 << 0);
constexpr int CONTACT_EXIT = (1 << 1);
constexpr int CONTACT_SKIM = (1 << 2);
constexpr int CONTACT_HIT  = (1 << 3);

struct tri_contact
{
    // Type of the contact.
    int type;

    // The ray parameter of intersection.
    double ray_t;

    // The two triangular parameters of intersection.
    double tri_s;
    double tri_t;

    // The point of intersection.
    ballistae::fixvec<double, 3> p;

    // The real (non-interpolated) normal of intersection.
    ballistae::fixvec<double, 3> n;
};

tri_contact tri_face_contact(
    const ballistae::ray_segment<double, 3> &r,
    const tri_face_crunched &f
) __attribute__((visibility("hidden")));

ballistae::contact<double> tri_mesh_contact(
    ballistae::ray_segment<double, 3> r,
    const ballistae::kd_tree<double, 3, tri_face_crunched> &mesh_kd_tree,
    const int want_type
) __attribute__((visibility("hidden")));

bool tri_mesh_sanity_check(const tri_mesh &the_mesh)
    __attribute__((visibility("hidden")));

#endif
