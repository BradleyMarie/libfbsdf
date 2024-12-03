#ifndef _LIBFBSDF_READERS_VALIDATING_BSDF_READER_
#define _LIBFBSDF_READERS_VALIDATING_BSDF_READER_

#include <cstdint>
#include <expected>
#include <string>
#include <utility>
#include <vector>

#include "libfbsdf/bsdf_reader.h"

namespace libfbsdf {

// A BSDF reader that performs
class ValidatingBsdfReader : public BsdfReader {
 protected:
  // Called at the start of parsing an input and passes information parsed from
  // the header of the BSDF file. Returns the parts of the file that should
  // be parsed or an error if the file cannot be read by the reader.
  virtual std::expected<Options, std::string> Start(
      const Flags& flags, uint32_t num_basis_functions,
      size_t num_color_channels, size_t longest_series_length,
      float index_of_refraction, float roughness_top,
      float roughness_bottom) = 0;

  // Provides an ordered list of the the elevational samples in one dimension.
  //
  // Will be called once per input.
  virtual void HandleElevationalSamples(std::vector<float> samples) {}

  // Provides the two dimensional CDF for each elevational sample.
  //
  // Will be called in order once per basis function in the input.
  virtual void HandleCdf(std::vector<float> values) {}

  // Provides the two dimensional bounds of the Fourier coefficients for each
  // elevational sample. The first element in each pair contains an offset into
  // the coefficients array and the second element in each pair contains the
  // number of coefficients in the Fourier series for that elevational sample.
  //
  // Will be called once per input.
  //
  // NOTE: For inputs with multiple basis functions or multiple color channels
  //       that the start index represents the index of the first color channel
  //       for the first basis function. The starting coefficients for the other
  //       color channels and basis functions can by offsetting this index by
  //       pair.second * (basis_function * num_color_channels + color_channel)
  virtual void HandleSeries(std::vector<std::pair<uint32_t, uint32_t>> series) {
  }

  // The list of Fourier coefficients stored in the input.
  //
  // Will be called once per input.
  virtual void HandleCoefficients(std::vector<float> coefficients) {}

  // TODO: Document what this contains
  //
  // Will be called once per input.
  virtual void HandleParameterSampleCounts(
      std::vector<uint32_t> sample_counts) {}

  // TODO: Document what this contains
  //
  // Will be called once per input.
  virtual void HandleParameterSamples(std::vector<float> samples) {}

 private:
  std::vector<float> elevational_samples_;
  std::vector<float> cdf_;
  std::vector<std::pair<uint32_t, uint32_t>> series_;
  std::vector<float> coefficients_;
  std::vector<uint32_t> parameter_sample_counts_;
  std::vector<float> parameter_samples_;
  uint32_t num_elevational_samples_1d_ = 0u;
  uint32_t num_elevational_samples_2d_ = 0u;
  uint32_t length_longest_series_ = 0u;
  uint32_t num_color_channels_ = 0u;
  uint32_t num_basis_functions_ = 0u;
  uint32_t num_coefficients_ = 0u;
  uint32_t num_parameters_ = 0u;
  uint32_t num_parameter_values_ = 0u;

 protected:
  // This class implements the entire BsdfReader interface with the exception of
  // HandleMetadata and Finish. Derived classes may implement those if desired.
  std::expected<Options, std::string> Start(
      const Flags& flags, size_t num_nodes, size_t num_basis_functions,
      size_t num_coefficients, size_t num_color_channels, size_t num_max_order,
      size_t num_parameters, size_t num_parameter_values,
      size_t metadata_size_bytes, float index_of_refraction,
      float roughness_top, float roughness_bottom) override final;

  std::expected<void, std::string> HandleElevationalSample(
      float value) override final;

  std::expected<void, std::string> HandleCdf(float value) override final;

  std::expected<void, std::string> HandleSeries(uint32_t offset,
                                                uint32_t length) override final;

  std::expected<void, std::string> HandleCoefficient(
      float value) override final;

  std::expected<void, std::string> HandleSampleCount(
      uint32_t value) override final;

  std::expected<void, std::string> HandleSamplePosition(
      float value) override final;
};

}  // namespace libfbsdf

#endif  // _LIBFBSDF_READERS_VALIDATING_BSDF_READER_