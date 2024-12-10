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
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "bsdf_reader",
    srcs = ["bsdf_reader.cc"],
    hdrs = ["bsdf_reader.h"],
    deps = [
        ":bsdf_header_reader",
    ],
)

cc_test(
    name = "bsdf_reader_test",
    srcs = ["bsdf_reader_test.cc"],
    deps = [
        ":bsdf_header_reader",
        ":test_bsdf_writer",
        "//test_data",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "test_bsdf_writer",
    testonly = 1,
    srcs = ["test_bsdf_writer.cc"],
    hdrs = ["test_bsdf_writer.h"],
)
