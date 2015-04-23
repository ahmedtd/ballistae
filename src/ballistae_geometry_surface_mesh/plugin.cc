#include <libballistae/geometry.hh>
#include <libguile_ballistae/geometry_plugin_interface.hh>

#include <utility>

#include <libballistae/kd_tree.hh>

#include "load_obj.hh"
#include "tri_mesh.hh"

namespace bl = ballistae;

class __attribute__((visibility("default")))
surface_mesh_priv : public ballistae::geometry
{
    bl::kd_tree<double, 3, tri_face_crunched> mesh;

public:
    surface_mesh_priv(const tri_mesh &mesh_in, size_t bucket_hint);
    virtual ~surface_mesh_priv();

    virtual bl::contact<double> ray_into(
        const bl::scene &the_scene,
        const bl::ray_segment<double,3> &query,
        std::ranlux24 &thread_rng
    ) const;

    virtual bl::contact<double> ray_exit(
        const bl::scene &the_scene,
        const bl::ray_segment<double,3> &query,
        std::ranlux24 &thread_rng
    ) const;
};

surface_mesh_priv::surface_mesh_priv(
    const tri_mesh &mesh_in,
    size_t bucket_hint
)
    : mesh(crunch(mesh_in, bucket_hint))
{
}

surface_mesh_priv::~surface_mesh_priv()
{
}

bl::contact<double> surface_mesh_priv::ray_into(
    const bl::scene &the_scene,
    const bl::ray_segment<double,3> &query,
    std::ranlux24 &thread_rng
) const
{
    return tri_mesh_contact(query, mesh, CONTACT_INTO);
}

bl::contact<double> surface_mesh_priv::ray_exit(
    const bl::scene &the_scene,
    const bl::ray_segment<double,3> &query,
    std::ranlux24 &thread_rng
) const
{
    return tri_mesh_contact(query, mesh, CONTACT_EXIT);
}

ballistae::geometry* guile_ballistae_geometry(SCM config_alist)
{
    scm_dynwind_begin((scm_t_dynwind_flags)0);

    SCM sym_file = scm_from_utf8_symbol("file");
    SCM sym_swapyz = scm_from_utf8_symbol("swapyz");
    SCM sym_bucket_hint = scm_from_utf8_symbol("bucket-hint");

    SCM lu_bucket_hint = scm_assq_ref(config_alist, sym_bucket_hint);
    size_t bucket_hint = scm_is_true(lu_bucket_hint)
        ? scm_to_double(lu_bucket_hint)
        : 96;

    bool swapyz = scm_is_true(scm_assq_ref(config_alist, sym_swapyz));

    SCM file_relpath = scm_assq_ref(config_alist, sym_file);
    if(scm_is_false(file_relpath))
        scm_wrong_type_arg(nullptr, 1, config_alist);

    SCM file_realpath = scm_call_1(
        scm_variable_ref(scm_c_lookup("bsta/asset-realpath")),
        file_relpath
    );

    char *filename = scm_to_utf8_stringn(file_realpath, nullptr);
    scm_dynwind_free(filename);

    int errc;
    size_t errline;
    tri_mesh the_mesh;
    std::tie(errc, errline, the_mesh) = tri_mesh_load_obj(filename, swapyz);

    switch(errc)
    {
    case OBJ_ERRC_FILE_NOT_LOADABLE:
        scm_misc_error(
            nullptr,
            "Failed while opening or mapping file ~A",
            scm_list_1(file_realpath)
        );
        break;
    case OBJ_ERRC_PARSE_ERROR:
        scm_misc_error(
            nullptr,
            "Parse failure in file ~A on line ~A",
            scm_list_2(file_realpath, scm_from_size_t(errline))
        );
        break;
    case OBJ_ERRC_INSANE:
        scm_misc_error(
            nullptr,
            "Surface mesh structure sanity check failed for file ~A",
            scm_list_1(file_realpath)
        );
        break;
    }

    scm_dynwind_end();

    return new surface_mesh_priv(std::move(the_mesh), bucket_hint);
}
