#ifndef _LIBFBSDF_BSDF_READER_
#define _LIBFBSDF_BSDF_READER_

#include <cstddef>
#include <cstdint>
#include <expected>
#include <istream>
#include <string>

namespace libfbsdf {

// The base class for reading BSDF files.
class BsdfReader {
 public:
  // NOTE: Behavior is undefined if input is not a binary stream
  std::expected<void, std::string> ReadFrom(std::istream& input);

 protected:
  struct Flags {
    bool is_bsdf;
    bool uses_harmonic_extrapolation;
  };

  struct Options {
    bool parse_elevational_samples = true;
    bool parse_parameter_sample_counts = true;
    bool parse_parameter_values = true;
    bool parse_cdf_mu = true;
    bool parse_series = true;
    bool parse_coefficients = true;
    bool parse_metadata = true;
  };

  virtual std::expected<Options, std::string> Start(
      const Flags& flags, size_t num_elevational_samples,
      size_t num_basis_functions, size_t num_coefficients,
      size_t num_color_channels, size_t longest_series_length,
      size_t num_parameters, size_t num_parameter_values,
      size_t metadata_size_bytes, float index_of_refraction,
      float roughness_top, float roughness_bottom) = 0;

  virtual std::expected<void, std::string> HandleElevationalSample(
      float value) {
    return std::expected<void, std::string>();
  }

  virtual std::expected<void, std::string> HandleSampleCount(uint32_t value) {
    return std::expected<void, std::string>();
  }

  virtual std::expected<void, std::string> HandleSamplePosition(float value) {
    return std::expected<void, std::string>();
  }

  virtual std::expected<void, std::string> HandleCdf(float value) {
    return std::expected<void, std::string>();
  }

  virtual std::expected<void, std::string> HandleSeries(uint32_t offset,
                                                        uint32_t length) {
    return std::expected<void, std::string>();
  }

  virtual std::expected<void, std::string> HandleCoefficient(float value) {
    return std::expected<void, std::string>();
  }

  virtual std::expected<void, std::string> HandleMetadata(std::string data) {
    return std::expected<void, std::string>();
  }

  virtual std::expected<void, std::string> Finish() {
    return std::expected<void, std::string>();
  }
};

}  // namespace libfbsdf

#endif  // _LIBFBSDF_BSDF_READER_