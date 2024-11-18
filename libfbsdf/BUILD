load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "bsdf_header_reader",
    srcs = ["bsdf_header_reader.cc"],
    hdrs = ["bsdf_header_reader.h"],
)

cc_test(
    name = "bsdf_header_reader_test",
    srcs = ["bsdf_header_reader_test.cc"],
    deps = [
        ":bsdf_header_reader",
        "@com_google_googletest//:gtest_main",
    ],
)
