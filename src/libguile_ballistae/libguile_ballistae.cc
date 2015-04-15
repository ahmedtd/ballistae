#include <tuple>
#include <vector>

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <libguile_ballistae/affine_transform.hh>
#include <libguile_ballistae/camera.hh>
#include <libguile_ballistae/dense_signal.hh>
#include <libguile_ballistae/geometry.hh>
#include <libguile_ballistae/illuminator.hh>
#include <libguile_ballistae/material.hh>
#include <libguile_ballistae/scene.hh>

namespace ballistae_guile
{

scm_t_bits smob_tag;
std::vector<subsmob_fns> subsmob_dispatch_table;

SCM ensure_smob(SCM obj, scm_t_bits flag)
{
    scm_assert_smob_type(smob_tag, obj);
    if(get_subsmob_type(obj) != flag)
        scm_wrong_type_arg(nullptr, SCM_ARG1, obj);
    return obj;
}

////////////////////////////////////////////////////////////////////////////////
/// Functions related to registration with a scene.
////////////////////////////////////////////////////////////////////////////////

bool is_registered(SCM obj)
{
    return SCM_SMOB_FLAGS(obj) & mask_registered;
}

SCM set_registered(SCM obj, bool registered)
{
    scm_t_bits bits = SCM_SMOB_FLAGS(obj);
    if(registered)
        bits = (bits | mask_registered) & 0xffff;
    else
        bits = (bits & ~mask_registered) & 0xffff;
    SCM_SET_SMOB_FLAGS(obj, bits);
    return obj;
}

////////////////////////////////////////////////////////////////////////////////
/// Functions for getting/setting subsmob type.
////////////////////////////////////////////////////////////////////////////////

scm_t_bits get_subsmob_type(SCM obj)
{
    return SCM_SMOB_FLAGS(obj) & mask_subsmob_type;
}

////////////////////////////////////////////////////////////////////////////////
/// Forwarders for essential subsmob functions.
////////////////////////////////////////////////////////////////////////////////

size_t smob_free(SCM obj)
{
    scm_t_bits flags = get_subsmob_type(obj);
    smob_free_t free_fn = subsmob_dispatch_table[flags].free_fn;
    return (*free_fn)(obj);
}

SCM smob_mark(SCM obj)
{
    scm_t_bits flags = get_subsmob_type(obj);
    smob_mark_t mark_fn = subsmob_dispatch_table[flags].mark_fn;
    return (*mark_fn)(obj);
}

int smob_print(SCM obj, SCM port, scm_print_state *pstate)
{
    scm_t_bits flags = get_subsmob_type(obj);
    smob_print_t print_fn = subsmob_dispatch_table[flags].print_fn;
    return (*print_fn)(obj, port, pstate);
}

SCM smob_equalp(SCM a, SCM b)
{
    // The equalp forwarder is a little more complex -- it needs to ensure the
    // two provided smobs are of the same subsmob type before dispatching.

    scm_t_bits flags_a = get_subsmob_type(a);
    scm_t_bits flags_b = get_subsmob_type(b);

    if(flags_a != flags_b)
    {
        return SCM_BOOL_F;
    }
    else
    {
        smob_equalp_t equalp_fn = subsmob_dispatch_table[flags_a].equalp_fn;
        return (*equalp_fn)(a, b);
    }
}

////////////////////////////////////////////////////////////////////////////////
/// End forwarders for essential subsmob functions.
////////////////////////////////////////////////////////////////////////////////

SCM ballistae_p(SCM obj)
{
    return scm_from_bool(SCM_SMOB_PREDICATE(smob_tag, obj));
}

}

extern "C" void libguile_ballistae_init()
{
    namespace bg = ballistae_guile;

    // Initialize the base smob type.
    //
    // The 0 parameter indicates that there is no automatically-managed memory
    // attached to the smob type.  We *must* then provide free/mark/print/equalp
    // functions for the smob.
    bg::smob_tag = scm_make_smob_type("ballistae", 0);

    scm_set_smob_free(  bg::smob_tag, &bg::smob_free  );
    scm_set_smob_mark(  bg::smob_tag, &bg::smob_mark  );
    scm_set_smob_print( bg::smob_tag, &bg::smob_print );
    scm_set_smob_equalp(bg::smob_tag, &bg::smob_equalp);
   
    bg::subsmob_dispatch_table = {
        bg::scene::init(),
        bg::camera::init(),
        bg::geometry::init(),
        bg::material::init(),
        bg::illuminator::init(),
        bg::affine_transform::init(),
        bg::dense_signal::init()
    };

    scm_c_define_gsubr(
        "ballistae?", 1, 0, 0,
        (scm_t_subr) ballistae_guile::ballistae_p
    );
    
    scm_c_export("ballistae?", nullptr);
}
