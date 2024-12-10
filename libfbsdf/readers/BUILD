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

cc_test(
    name = "validating_bsdf_reader_test",
    srcs = ["validating_bsdf_reader_test.cc"],
    deps = [
        ":validating_bsdf_reader",
        "//libfbsdf:test_bsdf_writer",
        "//test_data",
        "@googletest//:gtest_main",
    ],
)
