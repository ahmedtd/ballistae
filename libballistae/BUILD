load("@rules_proto//proto:defs.bzl", "proto_library")

cc_library(
    name = "libballistae",
    visibility = ["//visibility:public"],
    deps = [
        ":aabox",
        ":camera",
        ":color",
        ":contact",
        ":dense_signal",
        ":geometry",
        ":spectral_image",
        ":image_patch",
        ":kd_tree",
        ":material",
        ":material_map",
        ":options",
        ":ray",
        ":render_scene",
        ":scene",
        ":span",
        ":tetmesh",
        ":vector_distributions",
        ":vector",
        "//frustum/geometry:affine_transform",
        "//libballistae/camera:pinhole",
        "//libballistae/geometry",
        "//libballistae/material",
    ],
)

cc_library(
    name = "aabox",
    hdrs = ["aabox.hh"],
    deps = [
        ":ray",
        ":span",
        ":vector",
    ],
)

cc_library(
    name = "camera",
    hdrs = ["camera.hh"],
    deps = [
        ":ray",
        ":vector",
    ],
)

cc_library(
    name = "color",
    hdrs = ["color.hh"],
    deps = [
        ":dense_signal",
    ],
)

cc_library(
    name = "contact",
    hdrs = ["contact.hh"],
    deps = [
        ":ray",
        ":vector",
    ],
)

cc_library(
    name = "dense_signal",
    hdrs = ["dense_signal.hh"],
)

cc_library(
    name = "geometry",
    hdrs = ["geometry.hh"],
    deps = [
        ":aabox",
        ":contact",
        ":ray",
        ":span",
    ],
)

cc_library(
    name = "spectral_image",
    hdrs = ["spectral_image.hh"],
    srcs = ["spectral_image.cc"],
    deps = [
        ":spectral_image_file_cc_proto",
        "//third_party/zlib",
    ],
)

cc_library(
    name = "image_patch",
    hdrs = ["image_patch.hh"],
)

cc_library(
    name = "kd_tree",
    hdrs = ["kd_tree.hh"],
    deps = [
        ":aabox",
        ":span",
    ],
)

cc_library(
    name = "material",
    hdrs = ["material.hh"],
    deps = [
        ":contact",
        ":ray",
    ],
)

cc_library(
    name = "material_map",
    hdrs = ["material_map.hh"],
    deps = [
        ":dense_signal",
        ":vector",
        "//frustum/indicial",
    ],
)

cc_library(
    name = "options",
    hdrs = ["options.hh"],
    deps = [
        ":scene",
    ],
)

cc_library(
    name = "ray",
    hdrs = ["ray.hh"],
    deps = [
        ":span",
        ":vector",
        "//frustum/geometry:affine_transform",
    ],
)

cc_library(
    name = "render_scene",
    hdrs = ["render_scene.hh"],
    srcs = ["render_scene.cc"],
    deps = [
        ":camera",
        ":spectral_image",
        ":ray",
        ":scene",
        ":span",
    ],
    copts = ["-fopenmp"],
    linkopts = ["-lgomp"],
)

cc_library(
    name = "scene",
    hdrs = ["scene.hh"],
    srcs = ["scene.cc"],
    deps = [
        ":camera",
        ":color",
        ":contact",
        ":geometry",
        ":spectral_image",
        ":kd_tree",
        ":material",
        ":material_map",
        ":ray",
        ":span",
        ":vector",
        "//frustum/geometry:affine_transform",
    ],
)

cc_library(
    name = "span",
    hdrs = ["span.hh"],
)

cc_library(
    name = "tetmesh",
    hdrs = ["tetmesh.hh"],
    deps = [
        ":vector",
    ],
)

cc_library(
    name = "vector_distributions",
    hdrs = ["vector_distributions.hh"],
    deps = [
        ":vector",
    ],
)

cc_library(
    name = "vector",
    hdrs = ["vector.hh"],
    deps = [
        "//frustum/indicial",
    ],
)

proto_library(
    name = "spectral_image_file",
    srcs = ["spectral_image_file.proto"],
)

cc_proto_library(
    name = "spectral_image_file_cc_proto",
    deps = [":spectral_image_file"],
)
