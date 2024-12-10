#include "libfbsdf/test_bsdf_writer.h"

#include <algorithm>
#include <bit>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

namespace libfbsdf {
namespace testing {
namespace {

void WriteUInt32(std::string& output, uint32_t value) {
  if constexpr (std::endian::native != std::endian::little) {
    value = std::byteswap(value);
  }

  const char* bytes = reinterpret_cast<const char*>(&value);
  output += bytes[0];
  output += bytes[1];
  output += bytes[2];
  output += bytes[3];
}

void WriteFloat(std::string& output, float value) {
  WriteUInt32(output, std::bit_cast<uint32_t>(value));
}

}  // namespace

BsdfData::BsdfData(std::vector<float> elevational_samples,
                   size_t num_basis_functions, size_t num_channels)
    : elevational_samples_(std::move(elevational_samples)),
      series_(elevational_samples_.size() * elevational_samples_.size() *
              num_channels),
      cdf_(elevational_samples_.size() * elevational_samples_.size() *
               num_basis_functions,
           0.0f),
      num_basis_functions_(num_basis_functions),
      num_channels_(num_channels) {}

void BsdfData::AddCoefficient(size_t channel, size_t sample_x, size_t sample_y,
                              float value) {
  size_t start_index =
      elevational_samples_.size() * elevational_samples_.size() * channel;
  series_.at(start_index + sample_y * elevational_samples_.size() + sample_x)
      .push_back(value);
}

BsdfData::Coefficients BsdfData::SerializeCoefficients() const {
  BsdfData::Coefficients result;
  result.max_order = 0;
  for (const auto& list : series_) {
    result.bounds.push_back(result.bounds.size());
    result.bounds.push_back(list.size());
    result.coefficients.insert(result.coefficients.end(), list.begin(),
                               list.end());
    result.max_order = std::max(result.max_order, list.size());
  }
  return result;
}

void BsdfData::SetCdf(size_t basis_function, size_t sample_x, size_t sample_y,
                      float value) {
  size_t span_size = elevational_samples_.size() * elevational_samples_.size();
  cdf_.at(span_size * basis_function) = value;
}

std::string MakeBsdfFile(const Flags& flags, const BsdfData& bsdf_data,
                         std::vector<uint32_t> parameter_sample_counts,
                         std::vector<float> parameters, std::string metadata,
                         float index_of_refraction, float roughness_top,
                         float roughness_bottom) {
  BsdfData::Coefficients coefficients = bsdf_data.SerializeCoefficients();

  std::string result;

  // identifier
  result += 'S';
  result += 'C';
  result += 'A';
  result += 'T';
  result += 'F';
  result += 'U';
  result += 'N';

  // version
  result += '\1';

  // flags
  char is_bsdf = flags.is_bsdf ? '\1' : '\0';
  char uses_harmonic_extrapolation =
      flags.uses_harmonic_extrapolation ? '\2' : '\0';

  result += is_bsdf | uses_harmonic_extrapolation;
  result += '\0';
  result += '\0';
  result += '\0';

  // nNodes
  WriteUInt32(result, bsdf_data.GetElevationalSamples().size());

  // nCoeffs
  WriteUInt32(result, coefficients.coefficients.size());

  // nMaxOrder
  WriteUInt32(result, coefficients.max_order);

  // nChannels
  WriteUInt32(result, bsdf_data.GetNumChannels());

  // nBases
  WriteUInt32(result, bsdf_data.GetNumBasisFunctions());

  // nMetadataBytes
  WriteUInt32(result, metadata.size());

  // nParameters
  WriteUInt32(result, parameter_sample_counts.size());

  // nParameterValues
  WriteUInt32(result, parameters.size());

  // eta
  WriteFloat(result, index_of_refraction);

  // roughness
  WriteFloat(result, roughness_top);
  WriteFloat(result, roughness_bottom);

  // reserved
  result += '\0';
  result += '\0';
  result += '\0';
  result += '\0';
  result += '\0';
  result += '\0';
  result += '\0';
  result += '\0';

  for (float f : bsdf_data.GetElevationalSamples()) {
    WriteFloat(result, f);
  }

  for (uint32_t u : parameter_sample_counts) {
    WriteUInt32(result, u);
  }

  for (float f : parameters) {
    WriteFloat(result, f);
  }

  for (float f : bsdf_data.GetCdf()) {
    WriteFloat(result, f);
  }

  for (uint32_t u : coefficients.bounds) {
    WriteUInt32(result, u);
  }

  for (float f : coefficients.coefficients) {
    WriteFloat(result, f);
  }

  for (char c : metadata) {
    result += c;
  }

  return result;
}

std::string MakeEmptyBsdfFile(float index_of_refraction, float roughness_top,
                              float roughness_bottom) {
  BsdfData data(std::vector<float>(), 0, 0);

  Flags flags{.is_bsdf = true, .uses_harmonic_extrapolation = false};

  return MakeBsdfFile(flags, data, {}, {}, "", index_of_refraction,
                      roughness_top, roughness_bottom);
}

std::string MakeMinimalBsdfFile(float index_of_refraction, float roughness_top,
                                float roughness_bottom) {
  BsdfData data(std::vector<float>({1.0f}), 1, 1);
  data.AddCoefficient(0, 0, 0, 1.0f);
  data.SetCdf(0, 0, 0, 1.0f);

  Flags flags{.is_bsdf = true, .uses_harmonic_extrapolation = false};

  return MakeBsdfFile(flags, data, {1}, {1.0f}, "meta", index_of_refraction,
                      roughness_top, roughness_bottom);
}

std::string MakeNonFiniteBsdfFile(float index_of_refraction,
                                  float roughness_top, float roughness_bottom) {
  BsdfData data(std::vector<float>({std::numeric_limits<float>::quiet_NaN()}),
                1, 1);
  data.AddCoefficient(0, 0, 0, std::numeric_limits<float>::quiet_NaN());
  data.SetCdf(0, 0, 0, std::numeric_limits<float>::quiet_NaN());

  Flags flags{.is_bsdf = true, .uses_harmonic_extrapolation = false};

  return MakeBsdfFile(flags, data, {1},
                      {std::numeric_limits<float>::quiet_NaN()}, "meta",
                      index_of_refraction, roughness_top, roughness_bottom);
}

}  // namespace testing
}  // namespace libfbsdf