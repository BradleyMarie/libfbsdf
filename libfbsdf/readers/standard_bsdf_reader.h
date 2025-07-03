#ifndef _LIBFBSDF_READERS_STANDARD_BSDF_READER_
#define _LIBFBSDF_READERS_STANDARD_BSDF_READER_

#include <cstddef>
#include <cstdint>
#include <expected>
#include <istream>
#include <string>
#include <utility>
#include <vector>

namespace libfbsdf {

struct ReadFromStandardBsdfResult {
  std::vector<float> elevational_samples;
  std::vector<float> cdf;
  std::vector<float> y_coefficients;
  std::vector<float> r_coefficients;
  std::vector<float> b_coefficients;
  std::vector<std::pair<size_t, size_t>> series;
  float index_of_refraction;
  float roughness_top;
  float roughness_bottom;
};

// This function allows from reading from "standard" BSDF inputs (the common
// use case for rendering) without the need to derive from any of the BSDF
// reader types. In addition to the typical validation performed on inputs by
// `ValidatingBsdfReader`, this function also rejects inputs that do not have
// the following properties.
//
//  1) The BSDF bit in their header is set to true
//  2) The harmonic extrapolation bit in their header is set to false
//  3) Contain at least 3 elevational samples
//  4) Contain one or more basis functions (only the first will be used)
//  5) Have one or three color channels
//
// Additionally, for BSDF inputs containing three color channels, this function
// will also handle the process of de-interleaving the three channels so that
// each channel is stored separately and updating the series extents to match.
std::expected<ReadFromStandardBsdfResult, std::string> ReadFromStandardBsdf(
    std::istream& input);

}  // namespace libfbsdf

#endif  // _LIBFBSDF_READERS_VALIDATING_BSDF_READER_
