#include "libfbsdf/readers/pbrt_bsdf_reader.h"

#include <cstdint>
#include <expected>
#include <istream>
#include <limits>
#include <memory>
#include <string>
#include <vector>

namespace libfbsdf {

std::expected<BsdfReader::Options, std::string> PbrtBsdfReader::Start(
    const Flags& flags, size_t num_nodes, size_t num_basis_functions,
    size_t num_coefficients, size_t num_color_channels, size_t num_max_order,
    size_t num_parameters, size_t num_parameter_values,
    size_t metadata_size_bytes, float index_of_refraction, float alpha_top,
    float alpha_bottom) {
  mu_ = std::make_shared<std::vector<float>>();
  cdf_ = std::make_shared<std::vector<float>>();
  a_ = std::make_shared<std::vector<float>>();
  aOffset_ = std::make_shared<std::vector<uint32_t>>();
  m_ = std::make_shared<std::vector<uint32_t>>();
  a0_ = std::make_shared<std::vector<float>>();
  index_of_refraction_ = index_of_refraction;
  mMax_ = num_max_order;
  nChannels_ = num_color_channels;

  return BsdfReader::Options{
      .parse_nodes = true,
      .parse_parameter_sample_counts = false,
      .parse_parameter_values = false,
      .parse_cdf_mu = true,
      .parse_offset_table = true,
      .parse_coefficients = true,
      .parse_metadata = false,
  };
}

std::expected<void, std::string> PbrtBsdfReader::HandleNode(float value) {
  mu_->push_back(value);
  return std::expected<void, std::string>();
}

std::expected<void, std::string> PbrtBsdfReader::HandleCdfMu(float value) {
  cdf_->push_back(value);
  return std::expected<void, std::string>();
}

std::expected<void, std::string> PbrtBsdfReader::HandleOffsetAndLength(
    uint32_t offset, uint32_t length) {
  aOffset_->push_back(offset);
  m_->push_back(length);
  return std::expected<void, std::string>();
}

std::expected<void, std::string> PbrtBsdfReader::HandleCoefficient(
    float value) {
  a_->push_back(value);
  return std::expected<void, std::string>();
}

std::expected<void, std::string> PbrtBsdfReader::Finish() {
  for (size_t i = 0; i < aOffset_->size(); i++) {
    float value = 0.0f;
    if ((*m_)[i] != 0u) {
      value = (*a_)[i];
    }

    a0_->push_back(value);
  }

  mu_->shrink_to_fit();
  cdf_->shrink_to_fit();
  a_->shrink_to_fit();
  aOffset_->shrink_to_fit();
  m_->shrink_to_fit();
  a0_->shrink_to_fit();

  return std::expected<void, std::string>();
}

// This allows us to assume that uint32_t -> size_t conversions are not lossy
static_assert(std::numeric_limits<uint32_t>::max() <=
              std::numeric_limits<size_t>::max());

}  // namespace libfbsdf