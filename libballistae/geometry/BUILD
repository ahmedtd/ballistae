cc_library(
    name = "geometry",
    deps = [
        ":box",
        ":cylinder",
        ":infinity",
        ":load_obj",
        ":plane",
        ":sphere",
        ":surface_mesh",
        ":tri_mesh",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "box",
    hdrs = ["box.hh"],
)

cc_library(
    name = "cylinder",
    hdrs = ["cylinder.hh"],
)

cc_library(
    name = "infinity",
    hdrs = ["infinity.hh"],
)

cc_library(
    name = "load_obj",
    hdrs = ["load_obj.hh"],
    deps = [":tri_mesh"],
)

cc_library(
    name = "plane",
    hdrs = ["plane.hh"],
)

cc_library(
    name = "sphere",
    hdrs = ["sphere.hh"],
)

cc_library(
    name = "surface_mesh",
    hdrs = ["surface_mesh.hh"],
)

cc_library(
    name = "tri_mesh",
    hdrs = ["tri_mesh.hh"],
)
