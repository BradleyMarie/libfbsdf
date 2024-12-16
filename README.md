# libFBSDF

[![Test Status](https://github.com/BradleyMarie/libfbsdf/actions/workflows/c-cpp.yml/badge.svg?branch=main)](https://github.com/BradleyMarie/libfbsdf/actions/workflows/c-cpp.yml)
[![License](https://img.shields.io/badge/License-BSD_3--Clause-blue.svg)](https://github.com/BradleyMarie/libfbsdf/master/LICENSE)

A zero-dependency LayerLab Fourier BSDF reader for C++23. While there is no
formal definition of the Fourier BSDF format, informal documentation can be
found in the source code of [LayerLab](https://github.com/wjakob/layerlab/blob/3e5257e3076a7287d1da9bbd4ee3f05fe37d3ee3/src/storage.cpp).

## Getting Started

libFBSDF uses Bazel as its build system. If you are using Bazel as well, you can
import libFBSDF into your workspace by adding a snippet like the following into
your `MODULE.bazel` file.

```
bazel_dep(name = "libfbsdf")
git_override(
    module_name = "libfbsdf",
    remote = "https://github.com/bradleymarie/libfbsdf.git",
    commit = "bcdb2f9ddf987e6fef5c7963505e00f33809ebe1",
)
```

Note: You should update `commit` to reference the to the latest commit on the
main branch.

libFBSDF code is structured with the core modules residing in the `libfbsdf`
directory. `bsdf_reader` contains the parent `BsdfReader` class that contains
the logic for parsing out the contents of a Fourier BSDF input. This class does
very little validation of its own other than ensuring that tokens are valid
floats and that the input is long enough to contain all of the parameters
decribed in its header.

The `BsdfReader` class is designed for extension and exposes a small public API
as well as a protected API that derived classes must implement.

Also inside the `libfbsdf` directory is the `readers` directory. This directory
contains pre-implemented readers for BSDF inputs that do more validation than
the base `BsdfReader` class and reduce the amount of code clients would need to
implement.

Currently, this extra validation is provided by `ValidatingBsdfReader` which
layers on top of `BsdfReader` class and validates a number of of properties to
ensure that the input is well formed. Notably, this includes validation of the
ordering of the coordinates of the elevational sample and bounds checks on the
Fourier series described in the input. `ValidatingBsdfReader` also aggregates
the contents of the input into vectors since it is expected that most clients
of this library would want to do so anyways.

## Examples

Currently, there is no example code written for libFBSDF; however, since
libFBSDF has good unit test coverage you can use the test code as a reference
for how to work with the library.

Additionally, the code in the `readers` directory can be used as a reference
for working with the `BsdfReader` class directly.

It is expected that most clients will implement `ValidatingBsdfReader` instead
of working with `BsdfReader` directly.

## Versioning

libFBSDF currently is not strongly versioned and it is recommended that users
update to the latest commit from the main branch as often as is reasonably
possible.