#include "libfbsdf/bsdf_header_reader.h"

#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <istream>
#include <string_view>

namespace libfbsdf {
namespace {

std::string_view UnexpectedEOF() { return "Unexpected EOF"; }

std::expected<void, std::string_view> ParseUInt32(std::istream& input,
                                                  uint32_t* value) {
  if (!input.read(reinterpret_cast<char*>(value), sizeof(*value))) {
    return std::unexpected(UnexpectedEOF());
  }

  if constexpr (std::endian::native != std::endian::little) {
    *value = std::byteswap(*value);
  }

  return std::expected<void, std::string_view>();
}

std::expected<void, std::string_view> ParseFloat(std::istream& input,
                                                 float* value) {
  uint32_t bytes;
  if (!input.read(reinterpret_cast<char*>(&bytes), sizeof(bytes))) {
    return std::unexpected(UnexpectedEOF());
  }

  if constexpr (std::endian::native != std::endian::little) {
    bytes = std::byteswap(bytes);
  }

  *value = std::bit_cast<float>(bytes);

  return std::expected<void, std::string_view>();
}

bool ParseMagicString(std::istream& input) {
  char c;
  return input.get(c) && c == 'S' && input.get(c) && c == 'C' && input.get(c) &&
         c == 'A' && input.get(c) && c == 'T' && input.get(c) && c == 'F' &&
         input.get(c) && c == 'U' && input.get(c) && c == 'N';
}

std::expected<void, std::string_view> CheckVersion(std::istream& input,
                                                   BsdfHeader* header) {
  char c;
  if (!input.get(c)) {
    return std::unexpected(UnexpectedEOF());
  }

  if (c != 1) {
    return std::unexpected("Only BSDF version 1 is supported");
  }

  header->version = 1;

  return std::expected<void, std::string_view>();
}

std::expected<void, std::string_view> ParseFlags(std::istream& input,
                                                 BsdfHeader* header) {
  uint32_t flags;
  if (auto result = ParseUInt32(input, &flags); !result) {
    return result;
  }

  if (flags > 3u) {
    return std::unexpected("Reserved flags were set to non-zero values");
  }

  header->is_bsdf = flags & 1u;
  header->uses_harmonic_extrapolation = flags & 2u;

  return std::expected<void, std::string_view>();
}

std::expected<void, std::string_view> ParseIndexOfRefraction(
    std::istream& input, BsdfHeader* header) {
  if (auto result = ParseFloat(input, &header->index_of_refraction); !result) {
    return result;
  }

  if (!std::isfinite(header->index_of_refraction) ||
      header->index_of_refraction < 1.0f) {
    return std::unexpected("Invalid index of refraction");
  }

  return std::expected<void, std::string_view>();
}

std::expected<void, std::string_view> ParseRoughness(std::istream& input,
                                                     BsdfHeader* header) {
  if (auto result = ParseFloat(input, &header->roughness[0]); !result) {
    return result;
  }

  if (auto result = ParseFloat(input, &header->roughness[1]); !result) {
    return result;
  }

  if (!std::isfinite(header->roughness[0]) || header->roughness[0] < 0.0f ||
      !std::isfinite(header->roughness[1]) || header->roughness[1] < 0.0f) {
    return std::unexpected("Invalid value for roughness");
  }

  return std::expected<void, std::string_view>();
}

std::expected<void, std::string_view> CheckReservedBytes(std::istream& input) {
  uint32_t reserved;
  if (auto result = ParseUInt32(input, &reserved); !result) {
    return std::unexpected(result.error());
  }

  if (reserved != 0u) {
    return std::unexpected("Reserved bytes were set to non-zero values");
  }

  return std::expected<void, std::string_view>();
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
//     float reserved[2];
// };

std::expected<BsdfHeader, std::string_view> ReadBsdfHeader(
    std::istream& input) {
  if (input.fail()) {
    return std::unexpected("Bad stream passed");
  }

  BsdfHeader header;

  if (!ParseMagicString(input)) {
    return std::unexpected("The input must start with the magic string");
  }

  if (auto result = CheckVersion(input, &header); !result) {
    return std::unexpected(result.error());
  }

  if (auto result = ParseFlags(input, &header); !result) {
    return std::unexpected(result.error());
  }

  if (auto result = ParseUInt32(input, &header.num_elevational_samples);
      !result) {
    return std::unexpected(result.error());
  }

  if (auto result = ParseUInt32(input, &header.num_coefficients); !result) {
    return std::unexpected(result.error());
  }

  if (auto result = ParseUInt32(input, &header.length_longest_series);
      !result) {
    return std::unexpected(result.error());
  }

  if (auto result = ParseUInt32(input, &header.num_color_channels); !result) {
    return std::unexpected(result.error());
  }

  if (auto result = ParseUInt32(input, &header.num_basis_functions); !result) {
    return std::unexpected(result.error());
  }

  if (auto result = ParseUInt32(input, &header.num_metadata_bytes); !result) {
    return std::unexpected(result.error());
  }

  if (auto result = ParseUInt32(input, &header.num_parameters); !result) {
    return std::unexpected(result.error());
  }

  if (auto result = ParseUInt32(input, &header.num_parameter_values); !result) {
    return std::unexpected(result.error());
  }

  if (auto result = ParseIndexOfRefraction(input, &header); !result) {
    return std::unexpected(result.error());
  }

  if (auto result = ParseRoughness(input, &header); !result) {
    return std::unexpected(result.error());
  }

  if (auto result = CheckReservedBytes(input); !result) {
    return std::unexpected(result.error());
  }

  if (auto result = CheckReservedBytes(input); !result) {
    return std::unexpected(result.error());
  }

  return header;
}

}  // namespace libfbsdf