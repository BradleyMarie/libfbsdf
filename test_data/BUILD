load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "test_data",
    testonly = 1,
    srcs = ["test_data.cc"],
    hdrs = ["test_data.h"],
    data = [
        "ceramic.bsdf.gz",
        "coated_copper.bsdf.gz",
        "leather.bsdf.gz",
        "paint.bsdf.gz",
        "roughglass_alpha_0.2.bsdf.gz",
        "roughgold_alpha_0.2.bsdf.gz",
    ],
    visibility = ["//libfbsdf:__subpackages__"],
    deps = [
        "//libfbsdf:bsdf_reader",
        "@bazel_tools//third_party/zlib",
        "@bazel_tools//tools/cpp/runfiles",
    ],
)
