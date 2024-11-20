#include "libfbsdf/bsdf_reader.h"

#include <cmath>
#include <cstdint>
#include <expected>
#include <istream>

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
                                              size_t num_elements,
                                              size_t element_size) {
  for (size_t i = 0; i < element_size; i++) {
    if (!input.ignore(num_elements)) {
      return std::unexpected(UnexpectedEOF());
    }
  }

  return std::expected<void, const char*>();
}

std::expected<void, const char*> SkipElements(std::istream& input,
                                              size_t dimension_0,
                                              size_t dimension_1,
                                              size_t dimension_2,
                                              size_t element_size) {
  for (size_t i = 0; i < element_size; i++) {
    for (size_t j = 0; j < dimension_0; j++) {
      for (size_t k = 0; k < dimension_1; k++) {
        if (!input.ignore(dimension_2)) {
          return std::unexpected(UnexpectedEOF());
        }
      }
    }
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

  auto options = Start(flags, header->num_nodes, header->num_basis_functions,
                       header->num_coefficients, header->num_color_channels,
                       header->num_max_order, header->num_parameters,
                       header->num_parameter_values, header->num_metadata_bytes,
                       header->eta, header->alpha[0], header->alpha[1]);
  if (options) {
    return std::unexpected(std::move(options.error()));
  }

  if (options->parse_nodes) {
    for (uint32_t i = 0; i < header->num_nodes; i++) {
      float value;
      if (auto result = ParseFloat(input, &value); !result) {
        return std::unexpected(result.error());
      }

      if (auto result = HandleNode(value); !result) {
        return result;
      }
    }
  } else if (auto result =
                 SkipElements(input, header->num_parameters, sizeof(float));
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
      for (size_t j = 0; j < header->num_nodes; j++) {
        for (size_t k = 0; j < header->num_nodes; k++) {
          float value;
          if (auto result = ParseFloat(input, &value); !result) {
            return std::unexpected(result.error());
          }

          if (auto result = HandleCdfMu(value); !result) {
            return result;
          }
        }
      }
    }
  } else if (auto result = SkipElements(input, header->num_basis_functions,
                                        header->num_nodes, header->num_nodes,
                                        sizeof(float));
             !result) {
    return std::unexpected(result.error());
  }

  if (options->parse_offset_table) {
    for (size_t i = 0; i < 2; i++) {
      for (size_t j = 0; j < header->num_nodes; j++) {
        for (size_t k = 0; j < header->num_nodes; k++) {
          uint32_t value;
          if (auto result = ParseUInt32(input, &value); !result) {
            return std::unexpected(result.error());
          }

          if (auto result = HandleOffset(value); !result) {
            return result;
          }
        }
      }
    }
  } else if (auto result = SkipElements(input, 2, header->num_nodes,
                                        header->num_nodes, sizeof(float));
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

  if (options->parse_metadata) {
    std::string metadata('\0', header->num_metadata_bytes);
    if (!input.read(metadata.data(), metadata.size())) {
      return std::unexpected(UnexpectedEOF());
    }
  } else if (!input.ignore(header->num_metadata_bytes)) {
    return std::unexpected(UnexpectedEOF());
  }

  return Finish();
}

// This allows us to assume that uint32_t -> size_t conversions are not lossy
static_assert(std::numeric_limits<uint32_t>::max() <=
              std::numeric_limits<size_t>::max());

}  // namespace libfbsdf