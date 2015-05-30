#include <libballistae/geometry.hh>
#include <libguile_ballistae/geometry_plugin_interface.hh>

#include <utility>

#include <libballistae/kd_tree.hh>

#include "load_obj.hh"
#include "tri_mesh.hh"

using namespace frustum;
using namespace ballistae;

class __attribute__((visibility("default")))
surface_mesh_priv : public ballistae::geometry
{
    kd_tree<double, 3, tri_face_crunched> mesh;

public:
    surface_mesh_priv(const tri_mesh &mesh_in)
        __attribute__((visibility("default")));

    virtual ~surface_mesh_priv()
        __attribute__((visibility("default")));

    virtual aabox<double, 3> get_aabox();

    virtual void crush(const scene &the_scene, double time);

    virtual contact<double> ray_into(
        const scene &the_scene,
        const ray_segment<double,3> &query
    ) const __attribute__((visibility("default")));

    virtual contact<double> ray_exit(
        const scene &the_scene,
        const ray_segment<double,3> &query
    ) const __attribute__((visibility("default")));
};

surface_mesh_priv::surface_mesh_priv(const tri_mesh &mesh_in)
    : mesh(crunch(mesh_in))
{
}

surface_mesh_priv::~surface_mesh_priv()
{
}

aabox<double, 3> surface_mesh_priv::get_aabox()
{
    return mesh.nodes[0].bounds;
}

void surface_mesh_priv::crush(const scene &the_scene, double time)
{
}

contact<double> surface_mesh_priv::ray_into(
    const scene &the_scene,
    const ray_segment<double,3> &query
) const
{
    return tri_mesh_contact(query, mesh, CONTACT_INTO);
}

contact<double> surface_mesh_priv::ray_exit(
    const scene &the_scene,
    const ray_segment<double,3> &query
) const
{
    return tri_mesh_contact(query, mesh, CONTACT_EXIT);
}

// Declared with default visibility in libguile_ballistae /
// material_plugin_interface.hh
std::unique_ptr<geometry> guile_ballistae_geometry(
    scene *p_scene,
    SCM config_alist
)
{
    scm_dynwind_begin((scm_t_dynwind_flags)0);

    SCM sym_file = scm_from_utf8_symbol("file");
    SCM sym_swapyz = scm_from_utf8_symbol("swapyz");

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

    return std::make_unique<surface_mesh_priv>(std::move(the_mesh));
}
