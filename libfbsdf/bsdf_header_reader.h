#ifndef _LIBFBSDF_BSDF_HEADER_READER_
#define _LIBFBSDF_BSDF_HEADER_READER_

#include <cstdint>
#include <expected>
#include <istream>
#include <string_view>

namespace libfbsdf {

struct BsdfHeader final {
  // The version number
  uint8_t version;

  // Indicates if the input represents a BSDF
  bool is_bsdf;

  // Indicates if the BSDF coefficients use harmonic extrapolation
  bool uses_harmonic_extrapolation;

  // The number of samples in one dimenstion of the elevational discretization
  uint32_t num_elevational_samples;

  // The total number of Fourier series coefficients stored in the input
  uint32_t num_coefficients;

  // The length of the longest Fourier series occurring in the input
  uint32_t length_longest_series;

  // The number of color channels
  uint32_t num_color_channels;

  // The number of BSDF basis functions (relevant for texturing)
  uint32_t num_basis_functions;

  // The number of textured material parameters
  uint32_t num_parameters;

  // The total number of BSDF samples for all textured parameters
  uint32_t num_parameter_values;

  // The number of bytes of BSDF metadata in the input
  uint32_t num_metadata_bytes;

  // The index of refraction of the material
  float index_of_refraction;

  // The Beckmann-equivalent roughness for the top and bottom faces respectively
  float alpha[2];
};

// NOTE: Behavior is undefined if input is not a binary stream
std::expected<BsdfHeader, std::string_view> ReadBsdfHeader(std::istream& input);

}  // namespace libfbsdf

#endif  // _LIBFBSDF_BSDF_HEADER_READER_