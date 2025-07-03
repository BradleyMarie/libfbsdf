#include "libfbsdf/readers/standard_bsdf_reader.h"

#include <cstdint>
#include <expected>
#include <istream>
#include <string>
#include <utility>
#include <vector>

#include "libfbsdf/bsdf_reader.h"
#include "libfbsdf/readers/validating_bsdf_reader.h"

namespace libfbsdf {
namespace {

class StandardBsdfReader final : public ValidatingBsdfReader {
 public:
  std::vector<float> elevational_samples;
  std::vector<float> cdf;
  std::vector<std::pair<uint32_t, uint32_t>> interleaved_extents;
  std::vector<float> interleaved_coefficients;
  uint32_t num_color_channels;
  float index_of_refraction;
  float roughness_top;
  float roughness_bottom;

 private:
  std::expected<Options, std::string> Start(const Flags& flags,
                                            uint32_t num_basis_functions,
                                            size_t num_color_channels,
                                            float index_of_refraction,
                                            float roughness_top,
                                            float roughness_bottom) override;

  std::expected<void, std::string> HandleElevationalSamples(
      std::vector<float> samples) override;

  std::expected<void, std::string> HandleCdf(
      std::vector<float> values) override;

  std::expected<void, std::string> HandleSeries(
      std::vector<std::pair<uint32_t, uint32_t>> series) override;

  std::expected<void, std::string> HandleCoefficients(
      std::vector<float> coefficients) override;
};

std::expected<BsdfReader::Options, std::string> StandardBsdfReader::Start(
    const Flags& flags, uint32_t num_basis_functions, size_t num_color_channels,
    float index_of_refraction, float roughness_top, float roughness_bottom) {
  if (!flags.is_bsdf) {
    return std::unexpected("The input does not indicate that it is a BSDF");
  }

  if (flags.uses_harmonic_extrapolation) {
    return std::unexpected(
        "The input uses harmonic extrapolation which is unsupported");
  }

  if (num_basis_functions == 0) {
    return std::unexpected("The input does not contain any basis functions");
  }

  if (num_color_channels != 1 && num_color_channels != 3) {
    return std::unexpected(
        "The input must contain either 1 or 3 color channels");
  }

  this->num_color_channels = num_color_channels;
  this->index_of_refraction = index_of_refraction;
  this->roughness_top = roughness_top;
  this->roughness_bottom = roughness_bottom;

  return Options();
}

std::expected<void, std::string> StandardBsdfReader::HandleElevationalSamples(
    std::vector<float> samples) {
  elevational_samples = std::move(samples);

  return std::expected<void, std::string>();
}

std::expected<void, std::string> StandardBsdfReader::HandleCdf(
    std::vector<float> values) {
  if (cdf.empty()) {
    cdf = std::move(values);
  }

  return std::expected<void, std::string>();
}

std::expected<void, std::string> StandardBsdfReader::HandleSeries(
    std::vector<std::pair<uint32_t, uint32_t>> series) {
  interleaved_extents = std::move(series);
  return std::expected<void, std::string>();
}

std::expected<void, std::string> StandardBsdfReader::HandleCoefficients(
    std::vector<float> coefficients) {
  interleaved_coefficients = std::move(coefficients);
  return std::expected<void, std::string>();
}

}  // namespace

std::expected<ReadFromStandardBsdfResult, std::string> ReadFromStandardBsdf(
    std::istream& input) {
  StandardBsdfReader bsdf_reader;
  if (std::expected<void, std::string> error = bsdf_reader.ReadFrom(input);
      !error) {
    return std::unexpected(std::move(error.error()));
  }

  if (bsdf_reader.elevational_samples.size() < 3) {
    return std::unexpected(
        "The input must contain at least 3 elevational samples");
  }

  ReadFromStandardBsdfResult result;
  result.elevational_samples = std::move(bsdf_reader.elevational_samples);
  result.cdf = std::move(bsdf_reader.cdf);
  result.index_of_refraction = bsdf_reader.index_of_refraction;
  result.roughness_top = bsdf_reader.roughness_top;
  result.roughness_bottom = bsdf_reader.roughness_bottom;

  std::vector<float>* outputs[3] = {
      &result.y_coefficients, &result.r_coefficients, &result.b_coefficients};
  for (auto [start, length] : bsdf_reader.interleaved_extents) {
    result.series.emplace_back(result.y_coefficients.size(), length);

    size_t next_coefficient = start;
    for (uint32_t channel = 0; channel < bsdf_reader.num_color_channels;
         channel++) {
      for (size_t i = 0; i < length; i++) {
        outputs[channel]->push_back(
            bsdf_reader.interleaved_coefficients[next_coefficient++]);
      }
    }
  }

  return result;
}

}  // namespace libfbsdf
