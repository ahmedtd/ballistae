cc_library(
    name = "libballistae",
    hdrs = glob(["*.hh"]),
    visibility = ["//visibility:public"],
    deps = [
        "//include/frustum-0/geometry:affine_transform",
        "//include/libballistae/camera:pinhole",
        "//include/libballistae/geometry",
        "//include/libballistae/material",
    ],
    copts=["-fopenmp"],
    linkopts = ["-lgomp"],
)
