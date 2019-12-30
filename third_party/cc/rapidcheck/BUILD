cc_library(
    name = "rapidcheck",
    srcs = glob(["src/**/*.cpp"]),
    hdrs = glob([
        "include/**/*.h",
        "include/**/*.hpp",
    ]),
    includes = [
        "include",
        "include/rapidcheck",
    ],
    textual_hdrs = glob([
        "src/**/*.h",
        "src/**/*.hpp",
    ]),
    visibility = ["//visibility:public"],
)

cc_library(
    name = "gtest",
    hdrs = glob([
        "extras/gtest/include/**/*.h",
        "extras/gtest/include/**/*.hpp",
    ]),
    includes = ["extras/gtest/include"],
    visibility = ["//visibility:public"],
    deps = [":rapidcheck"],
)
