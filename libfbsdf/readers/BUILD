load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "pbrt_bsdf_reader",
    srcs = ["pbrt_bsdf_reader.cc"],
    hdrs = ["pbrt_bsdf_reader.h"],
    deps = [
        "//libfbsdf:bsdf_reader",
    ],
)
