#include "libfbsdf/bsdf_reader.h"

#include <bit>
#include <cmath>
#include <cstdint>
#include <expected>
#include <istream>
#include <limits>
#include <utility>

#include "libfbsdf/bsdf_header_reader.h"

namespace libfbsdf {
namespace {

const char* UnexpectedEOF() { return "Unexpected EOF"; }

std::expected<void, const char*> ParseUInt32(std::istream& input,
                                             uint32_t* value) {
  input.read(reinterpret_cast<char*>(value), sizeof(*value));
  if (!input) {
    return std::unexpected(UnexpectedEOF());
  }

  if constexpr (std::endian::native != std::endian::little) {
    *value = std::byteswap(*value);
  }

  return std::expected<void, const char*>();
}

std::expected<void, const char*> ParseFloat(std::istream& input, float* value) {
  uint32_t bytes;
  input.read(reinterpret_cast<char*>(&bytes), sizeof(bytes));
  if (!input) {
    return std::unexpected(UnexpectedEOF());
  }

  if constexpr (std::endian::native != std::endian::little) {
    bytes = std::byteswap(bytes);
  }

  *value = std::bit_cast<float>(bytes);

  if (!std::isfinite(*value)) {
    return std::unexpected("Input contained a non-finite floating point value");
  }

  return std::expected<void, const char*>();
}

std::expected<void, const char*> SkipElements(std::istream& input,
                                              size_t dimension_0,
                                              size_t element_size) {
  for (size_t i = 0; i < element_size; i++) {
    if (!input.seekg(dimension_0, std::ios_base::cur)) {
      return std::unexpected(UnexpectedEOF());
    }
  }

  return std::expected<void, const char*>();
}

std::expected<void, const char*> SkipElements(std::istream& input,
                                              size_t dimension_0,
                                              size_t dimension_1,
                                              size_t element_size) {
  for (size_t i = 0; i < element_size; i++) {
    SkipElements(input, dimension_0, dimension_1);
  }

  return std::expected<void, const char*>();
}

std::expected<void, const char*> SkipElements(std::istream& input,
                                              size_t dimension_0,
                                              size_t dimension_1,
                                              size_t dimension_2,
                                              size_t element_size) {
  for (size_t i = 0; i < element_size; i++) {
    SkipElements(input, dimension_0, dimension_1, dimension_2);
  }

  return std::expected<void, const char*>();
}

}  // namespace

std::expected<void, std::string> BsdfReader::ReadFrom(std::istream& input) {
  auto header = ReadBsdfHeader(input);
  if (!header) {
    return std::unexpected(std::string(header.error()));
  }

  Flags flags{
      .is_bsdf = header->is_bsdf,
      .uses_harmonic_extrapolation = header->uses_harmonic_extrapolation};

  auto options = Start(
      flags, header->num_elevational_samples, header->num_basis_functions,
      header->num_coefficients, header->num_color_channels,
      header->length_longest_series, header->num_parameters,
      header->num_parameter_values, header->num_metadata_bytes,
      header->index_of_refraction, header->roughness[0], header->roughness[1]);
  if (!options) {
    return std::unexpected(options.error());
  }

  if (options->parse_elevational_samples) {
    for (uint32_t i = 0; i < header->num_elevational_samples; i++) {
      float value;
      if (auto result = ParseFloat(input, &value); !result) {
        return std::unexpected(result.error());
      }

      if (auto result = HandleElevationalSample(value); !result) {
        return result;
      }
    }
  } else if (auto result = SkipElements(input, header->num_elevational_samples,
                                        sizeof(float));
             !result) {
    return std::unexpected(result.error());
  }

  if (options->parse_parameter_sample_counts) {
    for (uint32_t i = 0; i < header->num_parameters; i++) {
      uint32_t value;
      if (auto result = ParseUInt32(input, &value); !result) {
        return std::unexpected(result.error());
      }

      if (auto result = HandleSampleCount(value); !result) {
        return result;
      }
    }
  } else if (auto result =
                 SkipElements(input, header->num_parameters, sizeof(uint32_t));
             !result) {
    return std::unexpected(result.error());
  }

  if (options->parse_parameter_values) {
    for (uint32_t i = 0; i < header->num_parameter_values; i++) {
      float value;
      if (auto result = ParseFloat(input, &value); !result) {
        return std::unexpected(result.error());
      }

      if (auto result = HandleSamplePosition(value); !result) {
        return result;
      }
    }
  } else if (auto result = SkipElements(input, header->num_parameter_values,
                                        sizeof(float));
             !result) {
    return std::unexpected(result.error());
  }

  if (options->parse_cdf_mu) {
    for (size_t i = 0; i < header->num_basis_functions; i++) {
      for (size_t j = 0; j < header->num_elevational_samples; j++) {
        for (size_t k = 0; k < header->num_elevational_samples; k++) {
          float value;
          if (auto result = ParseFloat(input, &value); !result) {
            return std::unexpected(result.error());
          }

          if (auto result = HandleCdf(value); !result) {
            return result;
          }
        }
      }
    }
  } else if (auto result =
                 SkipElements(input, header->num_basis_functions,
                              header->num_elevational_samples,
                              header->num_elevational_samples, sizeof(float));
             !result) {
    return std::unexpected(result.error());
  }

  if (options->parse_series) {
    for (size_t i = 0; i < header->num_elevational_samples; i++) {
      for (size_t j = 0; j < header->num_elevational_samples; j++) {
        uint32_t offset;
        if (auto result = ParseUInt32(input, &offset); !result) {
          return std::unexpected(result.error());
        }

        uint32_t length;
        if (auto result = ParseUInt32(input, &length); !result) {
          return std::unexpected(result.error());
        }

        if (auto result = HandleSeries(offset, length); !result) {
          return result;
        }
      }
    }
  } else if (auto result = SkipElements(input, header->num_elevational_samples,
                                        header->num_elevational_samples,
                                        2 * sizeof(float));
             !result) {
    return std::unexpected(result.error());
  }

  if (options->parse_coefficients) {
    for (uint32_t i = 0; i < header->num_coefficients; i++) {
      float value;
      if (auto result = ParseFloat(input, &value); !result) {
        return std::unexpected(result.error());
      }

      if (auto result = HandleCoefficient(value); !result) {
        return result;
      }
    }
  } else if (auto result =
                 SkipElements(input, header->num_coefficients, sizeof(float));
             !result) {
    return std::unexpected(result.error());
  }

  if (options->parse_metadata && header->num_metadata_bytes != 0) {
    std::string metadata(header->num_metadata_bytes, '\0');
    if (!input.read(metadata.data(), header->num_metadata_bytes)) {
      return std::unexpected(UnexpectedEOF());
    }

    if (auto result = HandleMetadata(std::move(metadata)); !result) {
      return result;
    }
  } else if (!input.seekg(header->num_metadata_bytes, std::ios_base::cur)) {
    return std::unexpected(UnexpectedEOF());
  }

  return Finish();
}

// This allows us to assume that uint32_t -> size_t conversions are not lossy
static_assert(std::numeric_limits<uint32_t>::max() <=
              std::numeric_limits<size_t>::max());

}  // namespace libfbsdf