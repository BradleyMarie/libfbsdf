#include "libfbsdf/readers/validating_bsdf_reader.h"

#include <cstdint>
#include <expected>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace libfbsdf {

std::expected<BsdfReader::Options, std::string> ValidatingBsdfReader::Start(
    const BsdfReader::Flags& flags, size_t num_nodes,
    size_t num_basis_functions, size_t num_coefficients,
    size_t num_color_channels, size_t longest_series_length,
    size_t num_parameters, size_t num_parameter_values,
    size_t metadata_size_bytes, float index_of_refraction, float alpha_top,
    float alpha_bottom) {
  if (num_nodes > std::numeric_limits<size_t>::max() / num_nodes) {
    return std::unexpected("Input is too large to fit into memory");
  }

  num_elevational_samples_1d_ = num_nodes;
  num_elevational_samples_2d_ = num_nodes * num_nodes;
  length_longest_series_ = longest_series_length;
  num_basis_functions_ = num_basis_functions;
  num_coefficients_ = num_coefficients;
  num_parameters_ = num_parameters;
  num_parameter_values_ = num_parameter_values;

  return Start(flags, num_basis_functions, num_color_channels,
               longest_series_length, index_of_refraction, alpha_top,
               alpha_bottom);
}

std::expected<void, std::string> ValidatingBsdfReader::HandleElevationalSample(
    float value) {
  if (!elevational_samples_.empty() && elevational_samples_.back() >= value) {
    return std::unexpected(
        "Input contained improperly ordered elevational samples");
  }

  elevational_samples_.reserve(num_elevational_samples_1d_);
  elevational_samples_.push_back(value);

  if (elevational_samples_.size() == num_elevational_samples_1d_) {
    HandleElevationalSamples(std::move(elevational_samples_));
    elevational_samples_.clear();
  }

  return std::expected<void, std::string>();
}

std::expected<void, std::string> ValidatingBsdfReader::HandleCdf(float value) {
  if (value < 0.0f) {
    return std::unexpected("Input contained an invalid CDF value");
  }

  cdf_.reserve(num_elevational_samples_2d_);
  cdf_.push_back(value);

  if (cdf_.size() == num_elevational_samples_2d_) {
    HandleCdf(std::move(cdf_));
    cdf_.clear();
  }

  return std::expected<void, std::string>();
}

std::expected<void, std::string> ValidatingBsdfReader::HandleSeries(
    uint32_t offset, uint32_t length) {
  if (offset >= coefficients_.size()) {
    return std::unexpected("Input contained an offset that was out of bounds");
  }

  if (length > length_longest_series_) {
    return std::unexpected(
        "Input contained a series that was longer than the maximum length "
        "defined in the input");
  }

  if (coefficients_.size() < length || coefficients_.size() - length < offset) {
    return std::unexpected(
        "Input contained a series that extended out of bounds");
  }

  series_.reserve(num_elevational_samples_2d_);
  series_.emplace_back(offset, length);

  if (series_.size() == num_elevational_samples_2d_) {
    HandleSeries(std::move(series_));
    series_.clear();
  }

  return std::expected<void, std::string>();
}

std::expected<void, std::string> ValidatingBsdfReader::HandleCoefficient(
    float value) {
  coefficients_.reserve(num_coefficients_);
  coefficients_.push_back(value);

  if (coefficients_.size() == num_coefficients_) {
    HandleCoefficients(std::move(coefficients_));
    coefficients_.clear();
  }

  return std::expected<void, std::string>();
}

std::expected<void, std::string> ValidatingBsdfReader::HandleSampleCount(
    uint32_t value) {
  parameter_sample_counts_.reserve(num_parameters_);
  parameter_sample_counts_.push_back(value);

  if (parameter_sample_counts_.size() == num_parameters_) {
    HandleParameterSampleCounts(std::move(parameter_sample_counts_));
    parameter_sample_counts_.clear();
  }

  return std::expected<void, std::string>();
}

std::expected<void, std::string> ValidatingBsdfReader::HandleSamplePosition(
    float value) {
  parameter_samples_.reserve(num_parameter_values_);
  parameter_samples_.push_back(value);

  if (parameter_samples_.size() == num_parameter_values_) {
    HandleParameterSamples(std::move(parameter_samples_));
    parameter_samples_.clear();
  }

  return std::expected<void, std::string>();
}

// This allows us to assume that uint32_t -> size_t conversions are not lossy
static_assert(std::numeric_limits<uint32_t>::max() <=
              std::numeric_limits<size_t>::max());

}  // namespace libfbsdf