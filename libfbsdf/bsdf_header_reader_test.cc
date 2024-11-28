#include "libfbsdf/bsdf_header_reader.h"

#include <bit>
#include <fstream>
#include <limits>
#include <sstream>
#include <string>

#include "googletest/include/gtest/gtest.h"

namespace libfbsdf {
namespace {

std::string FloatToString(float value) {
  uint32_t word = std::bit_cast<uint32_t>(value);

  if constexpr (std::endian::native != std::endian::little) {
    word = std::byteswap(word);
  }

  const char* bytes = reinterpret_cast<const char*>(&word);

  std::string result;
  result += bytes[0];
  result += bytes[1];
  result += bytes[2];
  result += bytes[3];

  return result;
}

std::string MakeHeader(float eta, float alpha0, float alpha1) {
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
  result += '\3';
  result += '\0';
  result += '\0';
  result += '\0';

  // nNodes
  result += '\1';
  result += '\2';
  result += '\3';
  result += '\4';

  // nCoeffs
  result += '\1';
  result += '\2';
  result += '\3';
  result += '\4';

  // nMaxOrder
  result += '\1';
  result += '\2';
  result += '\3';
  result += '\4';

  // nChannels
  result += '\1';
  result += '\2';
  result += '\3';
  result += '\4';

  // nBases
  result += '\1';
  result += '\2';
  result += '\3';
  result += '\4';

  // nMetadataBytes
  result += '\1';
  result += '\2';
  result += '\3';
  result += '\4';

  // nParameters
  result += '\1';
  result += '\2';
  result += '\3';
  result += '\4';

  // nParameterValues
  result += '\1';
  result += '\2';
  result += '\3';
  result += '\4';

  // eta
  result += FloatToString(eta);

  // alpha
  result += FloatToString(alpha0);
  result += FloatToString(alpha1);

  // reserved
  result += '\0';
  result += '\0';
  result += '\0';
  result += '\0';
  result += '\0';
  result += '\0';
  result += '\0';
  result += '\0';

  return result;
}

TEST(BsdfHeaderReader, BadStream) {
  std::ifstream input("notarealfile.brdf", std::ios::in | std::ios::binary);
  auto result = ReadBsdfHeader(input);
  EXPECT_EQ("Bad stream passed", result.error());
}

TEST(BsdfHeaderReader, Succeeds) {
  std::stringstream input(MakeHeader(1.0, 1.0, 1.0));
  BsdfHeader result = ReadBsdfHeader(input).value();
  EXPECT_EQ(result.version, 1u);
  EXPECT_TRUE(result.is_bsdf);
  EXPECT_TRUE(result.uses_harmonic_extrapolation);
  EXPECT_EQ(result.num_elevational_samples, 0x04030201u);
  EXPECT_EQ(result.num_coefficients, 0x04030201u);
  EXPECT_EQ(result.length_longest_series, 0x04030201u);
  EXPECT_EQ(result.num_color_channels, 0x04030201u);
  EXPECT_EQ(result.num_basis_functions, 0x04030201u);
  EXPECT_EQ(result.num_parameters, 0x04030201u);
  EXPECT_EQ(result.num_parameter_values, 0x04030201u);
  EXPECT_EQ(result.num_metadata_bytes, 0x04030201u);
  EXPECT_EQ(result.index_of_refraction, 1.0f);
  EXPECT_EQ(result.alpha[0], 1.0f);
  EXPECT_EQ(result.alpha[1], 1.0f);
}

TEST(BsdfHeaderReader, BadHeader) {
  for (size_t i = 0; i < 7; i++) {
    std::string header = MakeHeader(1.0, 1.0, 1.0);
    header[0] = 'Z';
    std::stringstream input(header);

    EXPECT_EQ("The input must start with the magic string",
              ReadBsdfHeader(input).error());
  }
}

TEST(BsdfHeaderReader, UnexpectedEOF) {
  std::string good_header = MakeHeader(1.0, 1.0, 1.0);
  for (size_t i = 7; i < good_header.size(); i++) {
    std::string header = MakeHeader(1.0, 1.0, 1.0);
    header.erase(i, good_header.size());
    std::stringstream input(header);

    EXPECT_EQ("Unexpected EOF", ReadBsdfHeader(input).error());
  }
}

TEST(BsdfHeaderReader, BadVersion) {
  std::string header = MakeHeader(1.0, 1.0, 1.0);
  header[7] = '\0';
  std::stringstream input(header);

  EXPECT_EQ("Only BSDF version 1 is supported", ReadBsdfHeader(input).error());
}

TEST(BsdfHeaderReader, BadFlags) {
  std::string header = MakeHeader(1.0, 1.0, 1.0);
  header[8] = '\4';
  std::stringstream input(header);

  EXPECT_EQ("Reserved flags were set to non-zero values",
            ReadBsdfHeader(input).error());
}

TEST(BsdfHeaderReader, NegativeEta) {
  std::stringstream input(MakeHeader(-1.0f, 1.0f, 1.0f));
  EXPECT_EQ("Invalid index of refraction", ReadBsdfHeader(input).error());
}

TEST(BsdfHeaderReader, ZeroEta) {
  std::stringstream input(MakeHeader(0.0f, 1.0f, 1.0f));
  EXPECT_EQ("Invalid index of refraction", ReadBsdfHeader(input).error());
}

TEST(BsdfHeaderReader, LessThanOneEta) {
  std::stringstream input(MakeHeader(0.99f, 1.0f, 1.0f));
  EXPECT_EQ("Invalid index of refraction", ReadBsdfHeader(input).error());
}

TEST(BsdfHeaderReader, InfiniteEta) {
  std::stringstream input(
      MakeHeader(std::numeric_limits<float>::infinity(), 1.0f, 1.0f));
  EXPECT_EQ("Invalid index of refraction", ReadBsdfHeader(input).error());
}

TEST(BsdfHeaderReader, NaNEta) {
  std::stringstream input(
      MakeHeader(std::numeric_limits<float>::quiet_NaN(), 1.0f, 1.0f));
  EXPECT_EQ("Invalid index of refraction", ReadBsdfHeader(input).error());
}

TEST(BsdfHeaderReader, NegativeAlphaTop) {
  std::stringstream input(MakeHeader(1.0f, -1.0f, 1.0f));
  EXPECT_EQ("Invalid value for alpha", ReadBsdfHeader(input).error());
}

TEST(BsdfHeaderReader, ZeroAlphaTop) {
  std::stringstream input(MakeHeader(1.0f, 0.0f, 1.0f));
  EXPECT_EQ("Invalid value for alpha", ReadBsdfHeader(input).error());
}

TEST(BsdfHeaderReader, InfiniteAlphaTop) {
  std::stringstream input(
      MakeHeader(1.0f, std::numeric_limits<float>::infinity(), 1.0f));
  EXPECT_EQ("Invalid value for alpha", ReadBsdfHeader(input).error());
}

TEST(BsdfHeaderReader, NaNAlphaTop) {
  std::stringstream input(
      MakeHeader(1.0f, std::numeric_limits<float>::quiet_NaN(), 1.0f));
  EXPECT_EQ("Invalid value for alpha", ReadBsdfHeader(input).error());
}

TEST(BsdfHeaderReader, NegativeAlphaBottom) {
  std::stringstream input(MakeHeader(1.0f, 1.0, -1.0f));
  EXPECT_EQ("Invalid value for alpha", ReadBsdfHeader(input).error());
}

TEST(BsdfHeaderReader, ZeroAlphaBottom) {
  std::stringstream input(MakeHeader(1.0f, 1.0, 0.0f));
  EXPECT_EQ("Invalid value for alpha", ReadBsdfHeader(input).error());
}

TEST(BsdfHeaderReader, InfiniteAlphaBottom) {
  std::stringstream input(
      MakeHeader(1.0f, 1.0, std::numeric_limits<float>::infinity()));
  EXPECT_EQ("Invalid value for alpha", ReadBsdfHeader(input).error());
}

TEST(BsdfHeaderReader, NaNAlphaBottom) {
  std::stringstream input(
      MakeHeader(1.0f, 1.0, std::numeric_limits<float>::quiet_NaN()));
  EXPECT_EQ("Invalid value for alpha", ReadBsdfHeader(input).error());
}

TEST(BsdfHeaderReader, BadReservedBytes) {
  for (size_t i = 0; i < 8; i++) {
    std::string header = MakeHeader(1.0, 1.0, 1.0);
    header[header.size() - 1 - i] = '\1';
    std::stringstream input(header);

    EXPECT_EQ("Reserved bytes were set to non-zero values",
              ReadBsdfHeader(input).error());
  }
}

}  // namespace
}  // namespace libfbsdf