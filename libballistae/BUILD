load("@rules_proto//proto:defs.bzl", "proto_library")

package(default_visibility=["//spectral_converter:__pkg__", "//spectral_inspector:__pkg__"])

cc_library(
    name = "libballistae",
    copts = [
        "--std=c++17",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":aabox",
        ":camera",
        ":color",
        ":contact",
        ":dense_signal",
        ":geometry",
        ":image_patch",
        ":kd_tree",
        ":material",
        ":material_map",
        ":ray",
        ":render_scene",
        ":scene",
        ":span",
        ":spectral_image",
        ":tetmesh",
        ":vector",
        ":vector_distributions",
        "//frustum/geometry:affine_transform",
        "//libballistae/camera:pinhole",
        "//libballistae/geometry",
        "//libballistae/material",
    ],
)

cc_library(
    name = "aabox",
    hdrs = ["aabox.hh"],
    copts = [
        "--std=c++17",
    ],
    deps = [
        ":ray",
        ":span",
        ":vector",
    ],
)

cc_library(
    name = "camera",
    hdrs = ["camera.hh"],
    copts = [
        "--std=c++17",
    ],
    deps = [
        ":ray",
        ":vector",
    ],
)

cc_library(
    name = "color",
    srcs = ["color.cc"],
    hdrs = ["color.hh"],
    copts = [
        "--std=c++17",
    ],
    deps = [
        ":dense_signal",
    ],
)

cc_library(
    name = "contact",
    hdrs = ["contact.hh"],
    copts = [
        "--std=c++17",
    ],
    deps = [
        ":ray",
        ":vector",
    ],
)

cc_library(
    name = "dense_signal",
    hdrs = ["dense_signal.hh"],
    copts = [
        "--std=c++17",
    ],
)

cc_library(
    name = "geometry",
    hdrs = ["geometry.hh"],
    copts = [
        "--std=c++17",
    ],
    deps = [
        ":aabox",
        ":contact",
        ":ray",
        ":span",
    ],
)

cc_library(
    name = "spectral_image",
    srcs = ["spectral_image.cc"],
    hdrs = ["spectral_image.hh"],
    copts = [
        "--std=c++17",
    ],
    deps = [
        ":span",
        ":spectral_image_file_cc_proto",
        ":zipstream",
    ],
)

cc_test(
    name = "spectral_image_test",
    srcs = ["spectral_image_test.cc"],
    copts = [
        "--std=c++17",
    ],
    deps = [
        ":spectral_image",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "image_patch",
    hdrs = ["image_patch.hh"],
    copts = [
        "--std=c++17",
    ],
)

cc_library(
    name = "kd_tree",
    hdrs = ["kd_tree.hh"],
    copts = [
        "--std=c++17",
    ],
    deps = [
        ":aabox",
        ":span",
    ],
)

cc_library(
    name = "material",
    hdrs = ["material.hh"],
    copts = [
        "--std=c++17",
    ],
    deps = [
        ":contact",
        ":ray",
    ],
)

cc_library(
    name = "material_map",
    hdrs = ["material_map.hh"],
    copts = [
        "--std=c++17",
    ],
    deps = [
        ":dense_signal",
        ":vector",
        "//frustum/indicial",
    ],
)

cc_library(
    name = "ray",
    hdrs = ["ray.hh"],
    copts = [
        "--std=c++17",
    ],
    deps = [
        ":span",
        ":vector",
        "//frustum/geometry:affine_transform",
    ],
)

cc_library(
    name = "render_scene",
    srcs = ["render_scene.cc"],
    hdrs = ["render_scene.hh"],
    copts = [
        "--std=c++17",
    ],
    deps = [
        ":camera",
        ":ray",
        ":scene",
        ":span",
        ":spectral_image",
    ],
)

cc_library(
    name = "scene",
    srcs = ["scene.cc"],
    hdrs = ["scene.hh"],
    copts = [
        "--std=c++17",
    ],
    deps = [
        ":camera",
        ":color",
        ":contact",
        ":geometry",
        ":kd_tree",
        ":material",
        ":material_map",
        ":ray",
        ":span",
        ":spectral_image",
        ":vector",
        "//frustum/geometry:affine_transform",
    ],
)

cc_library(
    name = "span",
    hdrs = ["span.hh"],
    copts = [
        "--std=c++17",
    ],
)

cc_library(
    name = "tetmesh",
    hdrs = ["tetmesh.hh"],
    copts = [
        "--std=c++17",
    ],
    deps = [
        ":vector",
    ],
)

cc_library(
    name = "vector_distributions",
    hdrs = ["vector_distributions.hh"],
    copts = [
        "--std=c++17",
    ],
    deps = [
        ":vector",
    ],
)

cc_library(
    name = "vector",
    hdrs = ["vector.hh"],
    copts = [
        "--std=c++17",
    ],
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

cc_library(
    name = "zipstream",
    srcs = ["zipstream.cc"],
    hdrs = ["zipstream.hh"],
    copts = ["--std=c++17"],
    deps = [
        "//third_party/zlib",
    ],
)

cc_test(
    name = "zipstream_test",
    srcs = ["zipstream_test.cc"],
    copts = ["--std=c++17"],
    deps = [
        ":zipstream",
        "//third_party/cc/rapidcheck:gtest",
        "@googletest//:gtest_main",
    ],
)
