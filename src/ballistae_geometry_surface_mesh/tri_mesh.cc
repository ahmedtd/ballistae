#include "tri_mesh.hh"

#include <cmath>

#include <libballistae/aabox.hh>
#include <libballistae/kd_tree.hh>
#include <libballistae/ray.hh>
#include <libballistae/vector.hh>

namespace bl = ballistae;

tri_face_verts load_face_v(const tri_mesh &mesh, const tri_face_idx &idx)
{
    return {mesh.v[idx.vi[0]], mesh.v[idx.vi[1]], mesh.v[idx.vi[2]]};
}

tri_face_normals load_face_n(const tri_mesh &mesh, const tri_face_idx &idx)
{
    return {mesh.n[idx.ni[0]], mesh.n[idx.ni[1]], mesh.n[idx.ni[2]]};
}

tri_face_texcoords load_face_m(const tri_mesh &mesh, const tri_face_idx &idx)
{
    return {mesh.m[idx.mi[0]], mesh.m[idx.mi[1]], mesh.m[idx.mi[2]]};
}

ballistae::aabox<double, 3> get_aabox(const tri_face_crunched &f)
{
    using std::minmax_element;

    bl::fixvec<double, 3> v0 = f.v0;
    bl::fixvec<double, 3> v1 = f.v0 + f.u;
    bl::fixvec<double, 3> v2 = f.v0 + f.v;

    std::array<double, 3> x = {v0(0), v1(0), v2(0)};
    std::array<double, 3> y = {v0(1), v1(1), v2(1)};
    std::array<double, 3> z = {v0(2), v1(2), v2(2)};

    bl::aabox<double, 3> result = {
        bl::from_ptr_pair(minmax_element(x.begin(), x.end())),
        bl::from_ptr_pair(minmax_element(y.begin(), y.end())),
        bl::from_ptr_pair(minmax_element(z.begin(), z.end()))
    };

    return result;
}

bl::kd_tree<double, 3, tri_face_crunched> crunch(
    const tri_mesh &m,
    size_t bucket_hint
)
{
    using std::log;

    std::vector<tri_face_crunched> facets(m.f.size());

    for(size_t i = 0; i < m.f.size(); ++i)
    {
        using std::min;
        using std::max;

        tri_face_verts v = load_face_v(m, m.f[i]);
        tri_face_crunched cf;

        cf.v0 = v.v0;

        cf.u = v.v1 - v.v0;
        cf.v = v.v2 - v.v0;

        cf.n = arma::normalise(arma::cross(cf.u,cf.v));

        cf.uu = arma::dot(cf.u, cf.u);
        cf.uv = arma::dot(cf.u, cf.v);
        cf.vv = arma::dot(cf.v, cf.v);

        cf.recip_denom = 1.0 / (cf.uu * cf.vv - cf.uv * cf.uv);

        facets[i] = cf;
    }

    bl::kd_tree<double, 3, tri_face_crunched> result(
        std::begin(facets),
        std::end(facets),
        bucket_hint,
        get_aabox
    );

    return result;
}

tri_contact tri_face_contact(
    const bl::ray<double, 3> &r,
    const tri_face_crunched &f,
    const int want_type
)
{
    using arma::dot;

    double cosine = dot(f.n, r.slope);
    double offset = dot(f.n, r.point - f.v0);
    double ray_t = -offset / cosine;

    int contact_type = 0;
    if(cosine < 0.0)
        contact_type = CONTACT_INTO;
    else if(cosine > 0.0)
        contact_type = CONTACT_EXIT;
    else if(cosine == 0.0 && offset == 0.0)
        contact_type = CONTACT_SKIM;

    if(!(contact_type & want_type))
    {
        tri_contact result;
        result.type = contact_type;
        return result;
    }

    bl::fixvec<double, 3> p = bl::eval_ray(r, ray_t);
    bl::fixvec<double, 3> w = p - f.v0;

    double tri_s = (f.vv * dot(f.u,w) - dot(f.v,w) * f.uv) * f.recip_denom;
    double tri_t = (f.uu * dot(f.v,w) - dot(f.u,w) * f.uv) * f.recip_denom;

    if(bl::contains({0.0, 1.0}, tri_s)
       && bl::contains({0.0, 1.0}, tri_t)
       && bl::contains({0.0, 1.0}, (tri_s + tri_t)))
    {
        contact_type |= CONTACT_HIT;
    }

    return {contact_type, ray_t, tri_s, tri_t, p, f.n};
}

bl::contact<double> tri_mesh_contact(
    const bl::ray_segment<double, 3> &r,
    const bl::kd_tree<double, 3, tri_face_crunched> &mesh_kd_tree,
    const int want_type
)
{
    tri_contact least_contact;
    least_contact.ray_t = std::numeric_limits<double>::infinity();

    // The kd_tree uses selector to drive the search.
    auto selector = [&](const bl::aabox<double, 3> &bounds) -> bool {
        return ray_test(r, bounds);
    };

    // When any stored object is indicated suspected to be relevant,
    // the kd_tree will call computor on it.
    auto computor = [&](const tri_face_crunched &face) -> void {
        tri_contact c = tri_face_contact(r.the_ray, face, want_type);
        if((c.type & CONTACT_HIT)
           && bl::contains(r.the_segment, c.ray_t)
           && c.ray_t < least_contact.ray_t)
        {
            least_contact = c;
        }
    };

    mesh_kd_tree.query(selector, computor);

    bl::contact<double> result;
    if(least_contact.ray_t == std::numeric_limits<double>::infinity())
    {
        // If we didn't hit any triangles, we can bail now.
        result.t = std::numeric_limits<double>::quiet_NaN();
        return result;
    }

    result.t = least_contact.ray_t;
    result.r = r.the_ray;
    result.p = least_contact.p;
    result.n = least_contact.n;
    result.uvw = least_contact.p;

    // Here, we could compute a different normal by interpolating the
    // mesh-specified normals.  Right now, we just use the computed normal of
    // the face.

    // Here, we would interpolate the mesh-specified uv coordinates.

    return result;
}

bool tri_mesh_sanity_check(const tri_mesh &the_mesh)
{
    bool normal_found_invalid_idx = false;
    bool normal_found_valid_idx = false;
    bool texcoord_found_invalid_idx = false;
    bool texcoord_found_valid_idx = false;
    for(const tri_face_idx &idx : the_mesh.f)
    {
        for(size_t i = 0; i < 3; ++i)
        {
            if(idx.vi[i] >= the_mesh.v.size())
                return false;

            if(idx.ni[i] < the_mesh.n.size())
                normal_found_valid_idx = true;
            else if(idx.ni[i] == std::numeric_limits<size_t>::max())
                normal_found_invalid_idx = true;
            else
                return false;

            if(idx.mi[i] < the_mesh.m.size())
                texcoord_found_valid_idx = true;
            else if(idx.mi[i] == std::numeric_limits<size_t>::max())
                texcoord_found_invalid_idx = true;
            else
                return false;
        }
    }

    if(normal_found_valid_idx && normal_found_invalid_idx)
        return false;
    if(texcoord_found_valid_idx && texcoord_found_invalid_idx)
        return false;

    return true;
}
