#include "libfbsdf/readers/validating_bsdf_reader.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace libfbsdf {
namespace {

std::expected<void, std::string> ValidateElevationalSamples(
    const std::vector<float> samples, float value,
    bool allow_duplicates_at_origin, bool& zero_duplicate_already_allowed) {
  if (value < -1.0f || value > 1.0f) {
    return std::unexpected(
        "Input contained elevational samples that were out of range");
  }

  if (samples.empty()) {
    return std::expected<void, std::string>();
  }

  if (samples.back() < value) {
    return std::expected<void, std::string>();
  }

  // A single duplicate value is allowed at the origin
  if (allow_duplicates_at_origin && samples.back() == value && value == 0.0f &&
      !zero_duplicate_already_allowed) {
    zero_duplicate_already_allowed = true;
    return std::expected<void, std::string>();
  }

  return std::unexpected(
      "Input contained improperly ordered elevational samples");
}

}  // namespace

std::expected<BsdfReader::Options, std::string> ValidatingBsdfReader::Start(
    const BsdfReader::Flags& flags, size_t num_elevational_samples,
    size_t num_basis_functions, size_t num_coefficients,
    size_t num_color_channels, size_t longest_series_length,
    size_t num_parameters, size_t num_parameter_values,
    size_t metadata_size_bytes, float index_of_refraction, float roughness_top,
    float roughness_bottom) {
  num_coefficients_per_length_ = num_basis_functions * num_color_channels;

  if ((num_elevational_samples != 0 &&
       num_elevational_samples >
           std::numeric_limits<size_t>::max() / num_elevational_samples) ||
      (num_basis_functions != 0 && num_color_channels != 0 &&
       num_coefficients_per_length_ / num_color_channels !=
           num_basis_functions)) {
    return std::unexpected("Input is too large to fit into memory");
  }

  num_elevational_samples_1d_ = num_elevational_samples;
  num_elevational_samples_2d_ =
      num_elevational_samples * num_elevational_samples;
  length_longest_series_ = longest_series_length;
  num_basis_functions_ = num_basis_functions;
  num_coefficients_ = num_coefficients;
  num_parameters_ = num_parameters;
  num_parameter_values_ = num_parameter_values;
  zero_duplicate_already_allowed_ = false;

  return Start(flags, num_basis_functions, num_color_channels,
               index_of_refraction, roughness_top, roughness_bottom);
}

std::expected<void, std::string> ValidatingBsdfReader::HandleElevationalSample(
    float value) {
  if (auto valid = ValidateElevationalSamples(
          elevational_samples_, value, options_.allow_duplicates_at_origin,
          zero_duplicate_already_allowed_);
      !valid) {
    return valid;
  }

  elevational_samples_.reserve(num_elevational_samples_1d_);
  elevational_samples_.push_back(value);

  std::expected<void, std::string> result;
  if (elevational_samples_.size() == num_elevational_samples_1d_) {
    result = HandleElevationalSamples(std::move(elevational_samples_));
    elevational_samples_.clear();
  }

  return result;
}

std::expected<void, std::string> ValidatingBsdfReader::HandleCdf(float value) {
  if (options_.clamp_cdf) {
    value = std::clamp(value, 0.0f, 1.0f);
  } else if (value < 0.0f || value > 1.0f) {
    return std::unexpected("Input contained a CDF value that was out of range");
  }

  if (cdf_.empty() && value != 0.0f) {
    return std::unexpected(
        "Input contained a CDF range that did not start with zero");
  }

  cdf_.reserve(num_elevational_samples_2d_);
  cdf_.push_back(value);

  std::expected<void, std::string> result;
  if (cdf_.size() == num_elevational_samples_2d_) {
    result = HandleCdf(std::move(cdf_));
    cdf_.clear();
  }

  return result;
}

std::expected<void, std::string> ValidatingBsdfReader::HandleSeries(
    uint32_t offset, uint32_t length) {
  if (length != 0u && offset >= num_coefficients_) {
    return std::unexpected("Input contained an offset that was out of bounds");
  }

  if (!options_.ignore_longest_series_length &&
      length > length_longest_series_) {
    return std::unexpected(
        "Input contained a series that was longer than the maximum length "
        "defined in the input");
  }

  size_t series_length = num_coefficients_per_length_ * length;
  if (series_length / num_coefficients_per_length_ != length) {
    return std::unexpected("Input is too large to fit into memory");
  }

  if (num_coefficients_ < series_length ||
      (series_length != 0u && num_coefficients_ - series_length < offset)) {
    return std::unexpected(
        "Input contained a series that extended out of bounds");
  }

  series_.reserve(num_elevational_samples_2d_);
  series_.emplace_back(offset, length);

  std::expected<void, std::string> result;
  if (series_.size() == num_elevational_samples_2d_) {
    result = HandleSeries(std::move(series_));
    series_.clear();
  }

  return result;
}

std::expected<void, std::string> ValidatingBsdfReader::HandleCoefficient(
    float value) {
  coefficients_.reserve(num_coefficients_);
  coefficients_.push_back(value);

  std::expected<void, std::string> result;
  if (coefficients_.size() == num_coefficients_) {
    result = HandleCoefficients(std::move(coefficients_));
    coefficients_.clear();
  }

  return result;
}

std::expected<void, std::string> ValidatingBsdfReader::HandleSampleCount(
    uint32_t value) {
  parameter_sample_counts_.reserve(num_parameters_);
  parameter_sample_counts_.push_back(value);

  std::expected<void, std::string> result;
  if (parameter_sample_counts_.size() == num_parameters_) {
    result = HandleParameterSampleCounts(std::move(parameter_sample_counts_));
    parameter_sample_counts_.clear();
  }

  return result;
}

std::expected<void, std::string> ValidatingBsdfReader::HandleSamplePosition(
    float value) {
  parameter_samples_.reserve(num_parameter_values_);
  parameter_samples_.push_back(value);

  std::expected<void, std::string> result;
  if (parameter_samples_.size() == num_parameter_values_) {
    result = HandleParameterSamples(std::move(parameter_samples_));
    parameter_samples_.clear();
  }

  return result;
}

// This allows us to assume that uint32_t -> size_t conversions are not lossy
static_assert(std::numeric_limits<uint32_t>::max() <=
              std::numeric_limits<size_t>::max());

}  // namespace libfbsdf
