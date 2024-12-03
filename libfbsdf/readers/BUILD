load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "validating_bsdf_reader",
    srcs = ["validating_bsdf_reader.cc"],
    hdrs = ["validating_bsdf_reader.h"],
    deps = [
        "//libfbsdf:bsdf_reader",
    ],
)
