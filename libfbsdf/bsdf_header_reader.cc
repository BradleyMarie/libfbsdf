#include "libfbsdf/bsdf_header_reader.h"

#include <cmath>
#include <cstdint>
#include <expected>
#include <istream>
#include <string>

namespace libfbsdf {
namespace {

std::string UnexpectedEOF() { return "Unexpected EOF"; }

std::expected<void, std::string> ParseUInt32(std::istream& input,
                                             uint32_t* value) {
  input.read(reinterpret_cast<char*>(value), sizeof(*value));
  if (!input) {
    return std::unexpected(UnexpectedEOF());
  }

  if constexpr (std::endian::native != std::endian::little) {
    *value = std::byteswap(*value);
  }

  return std::expected<void, std::string>();
}

std::expected<void, std::string> ParseFloat(std::istream& input, float* value) {
  uint32_t bytes;
  input.read(reinterpret_cast<char*>(&bytes), sizeof(bytes));
  if (!input) {
    return std::unexpected(UnexpectedEOF());
  }

  if constexpr (std::endian::native != std::endian::little) {
    bytes = std::byteswap(bytes);
  }

  *value = std::bit_cast<float>(bytes);

  return std::expected<void, std::string>();
}

bool ParseMagicString(std::istream& input) {
  char c;
  return input.get(c) && c == 'S' && input.get(c) && c == 'C' && input.get(c) &&
         c == 'A' && input.get(c) && c == 'T' && input.get(c) && c == 'F' &&
         input.get(c) && c == 'U' && input.get(c) && c == 'N';
}

std::expected<void, std::string> CheckVersion(std::istream& input,
                                              BsdfHeader* header) {
  char c;
  if (!input.get(c) || c != 1) {
    return std::unexpected("Only BSDF version 1 is supported");
  }

  header->version = 1;

  return std::expected<void, std::string>();
}

std::expected<void, std::string> ParseFlags(std::istream& input,
                                            BsdfHeader* header) {
  uint32_t flags;
  if (auto error = ParseUInt32(input, &flags); !error) {
    return error;
  }

  if (flags & 0xFFFFFFFCu) {
    return std::unexpected("Reserved flags were set to non-zero values");
  }

  header->is_bsdf = flags & 1u;
  header->is_harmonic_extrapolation = flags & 2u;

  return std::expected<void, std::string>();
}

std::expected<void, std::string> ParseEta(std::istream& input,
                                          BsdfHeader* header) {
  if (auto error = ParseFloat(input, &header->eta); !error) {
    return error;
  }

  if (!std::isfinite(header->eta) || header->eta < 1.0f) {
    return std::unexpected("Invalid value for eta");
  }

  return std::expected<void, std::string>();
}

std::expected<void, std::string> ParseAlpha(std::istream& input,
                                            BsdfHeader* header) {
  if (auto error = ParseFloat(input, &header->alpha[0]); !error) {
    return error;
  }

  if (auto error = ParseFloat(input, &header->alpha[1]); !error) {
    return error;
  }

  if (!std::isfinite(header->alpha[0]) || header->alpha[0] <= 0.0f ||
      !std::isfinite(header->alpha[1]) || header->alpha[1] <= 0.0f) {
    return std::unexpected("Invalid value for alpha");
  }

  return std::expected<void, std::string>();
}

std::expected<void, std::string> CheckReservedBytes(std::istream& input) {
  uint32_t reserved;
  if (auto error = ParseUInt32(input, &reserved); !error) {
    return std::unexpected(error.error());
  }

  if (!reserved != 0u) {
    return std::unexpected("Reserved bytes were set to non-zero values");
  }

  return std::expected<void, std::string>();
}

}  // namespace

//
// Per the layerlab source code, this is the structure of the header of a
// Fourier BSDF file. Values are stored in their little-endian representation.
//
// struct Header {
//     uint8_t identifier[7];
//     uint8_t version;
//     uint32_t flags;
//     uint32_t nNodes;
//     uint32_t nCoeffs;
//     uint32_t nMaxOrder;
//     uint32_t nChannels;
//     uint32_t nBases;
//     uint32_t nMetadataBytes;
//     uint32_t nParameters;
//     uint32_t nParameterValues;
//     float eta;
//     float alpha[2];
//     float unused[2];
// };

std::expected<BsdfHeader, std::string> ReadBsdfHeader(std::istream& input) {
  BsdfHeader result;

  if (!ParseMagicString(input)) {
    return std::unexpected("The input must start with the magic string");
  }

  if (auto error = CheckVersion(input, &result); !error) {
    return std::unexpected(error.error());
  }

  if (auto error = ParseFlags(input, &result); !error) {
    return std::unexpected(error.error());
  }

  if (auto error = ParseUInt32(input, &result.num_nodes); !error) {
    return std::unexpected(error.error());
  }

  if (auto error = ParseUInt32(input, &result.num_coefficients); !error) {
    return std::unexpected(error.error());
  }

  if (auto error = ParseUInt32(input, &result.num_max_order); !error) {
    return std::unexpected(error.error());
  }

  if (auto error = ParseUInt32(input, &result.num_color_channels); !error) {
    return std::unexpected(error.error());
  }

  if (auto error = ParseUInt32(input, &result.num_basis_functions); !error) {
    return std::unexpected(error.error());
  }

  if (auto error = ParseUInt32(input, &result.num_metadata_bytes); !error) {
    return std::unexpected(error.error());
  }

  if (auto error = ParseUInt32(input, &result.num_parameters); !error) {
    return std::unexpected(error.error());
  }

  if (auto error = ParseUInt32(input, &result.num_parameter_values); !error) {
    return std::unexpected(error.error());
  }

  if (auto error = ParseEta(input, &result); !error) {
    return std::unexpected(error.error());
  }

  if (auto error = ParseAlpha(input, &result); !error) {
    return std::unexpected(error.error());
  }

  if (auto error = CheckReservedBytes(input); !error) {
    return std::unexpected(error.error());
  }

  if (auto error = CheckReservedBytes(input); !error) {
    return std::unexpected(error.error());
  }

  return result;
}

}  // namespace libfbsdf