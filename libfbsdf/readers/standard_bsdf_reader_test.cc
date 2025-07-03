#include "libfbsdf/readers/standard_bsdf_reader.h"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <istream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "googlemock/include/gmock/gmock.h"
#include "googletest/include/gtest/gtest.h"
#include "libfbsdf/test_bsdf_writer.h"
#include "test_data/test_data.h"

namespace libfbsdf {
namespace {

using ::libfbsdf::testing::BsdfData;
using ::libfbsdf::testing::FileParams;
using ::libfbsdf::testing::Flags;
using ::libfbsdf::testing::kTestDataFiles;
using ::libfbsdf::testing::MakeEmptyBsdfFile;
using ::libfbsdf::testing::MakeMinimalBsdfFile;
using ::libfbsdf::testing::OpenTestData;
using ::testing::ElementsAre;
using ::testing::IsEmpty;

TEST(StandardBsdfReader, NotABsdf) {
  BsdfData data(std::vector<float>({0.0f}), 1, 1);
  Flags flags{.is_bsdf = false, .uses_harmonic_extrapolation = false};
  std::string bsdf_file_bytes =
      MakeBsdfFile(flags, data, {}, {}, "", 1.0f, 1.0f, 1.0f);
  std::stringstream stream(bsdf_file_bytes);

  auto result = ReadFromStandardBsdf(stream);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error(), "The input does not indicate that it is a BSDF");
}

TEST(StandardBsdfReader, UsesHarmonicExtrapolation) {
  BsdfData data(std::vector<float>({0.0f}), 1, 1);
  Flags flags{.is_bsdf = true, .uses_harmonic_extrapolation = true};
  std::string bsdf_file_bytes =
      MakeBsdfFile(flags, data, {}, {}, "", 1.0f, 1.0f, 1.0f);
  std::stringstream stream(bsdf_file_bytes);

  auto result = ReadFromStandardBsdf(stream);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error(),
            "The input uses harmonic extrapolation which is unsupported");
}

TEST(ValidatingBsdfReader, NoBasisFunctions) {
  BsdfData data(std::vector<float>({0.0f}), 0, 1);
  Flags flags{.is_bsdf = true, .uses_harmonic_extrapolation = false};

  std::string bsdf_file_bytes =
      MakeBsdfFile(flags, data, {}, {}, "", 1.0f, 1.0f, 1.0f);
  std::stringstream stream(bsdf_file_bytes);

  auto result = ReadFromStandardBsdf(stream);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error(), "The input does not contain any basis functions");
}

TEST(ValidatingBsdfReader, NoColorChannels) {
  BsdfData data(std::vector<float>({0.0f}), 1, 0);
  Flags flags{.is_bsdf = true, .uses_harmonic_extrapolation = false};

  std::string bsdf_file_bytes =
      MakeBsdfFile(flags, data, {}, {}, "", 1.0f, 1.0f, 1.0f);
  std::stringstream stream(bsdf_file_bytes);

  auto result = ReadFromStandardBsdf(stream);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error(),
            "The input must contain either 1 or 3 color channels");
}

TEST(ValidatingBsdfReader, TwoColorChannels) {
  BsdfData data(std::vector<float>({0.0f}), 1, 2);
  Flags flags{.is_bsdf = true, .uses_harmonic_extrapolation = false};

  std::string bsdf_file_bytes =
      MakeBsdfFile(flags, data, {}, {}, "", 1.0f, 1.0f, 1.0f);
  std::stringstream stream(bsdf_file_bytes);

  auto result = ReadFromStandardBsdf(stream);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error(),
            "The input must contain either 1 or 3 color channels");
}

TEST(ValidatingBsdfReader, FourColorChannels) {
  BsdfData data(std::vector<float>({0.0f}), 1, 4);
  Flags flags{.is_bsdf = true, .uses_harmonic_extrapolation = false};

  std::string bsdf_file_bytes =
      MakeBsdfFile(flags, data, {}, {}, "", 1.0f, 1.0f, 1.0f);
  std::stringstream stream(bsdf_file_bytes);

  auto result = ReadFromStandardBsdf(stream);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error(),
            "The input must contain either 1 or 3 color channels");
}

TEST(ValidatingBsdfReader, TooFewElevationalSamples0) {
  BsdfData data(std::vector<float>(), 1, 1);

  Flags flags{.is_bsdf = true, .uses_harmonic_extrapolation = false};

  std::string bsdf_file_bytes =
      MakeBsdfFile(flags, data, {}, {}, "", 1.0f, 1.0f, 1.0f);
  std::stringstream stream(bsdf_file_bytes);

  auto result = ReadFromStandardBsdf(stream);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error(),
            "The input must contain at least 3 elevational samples");
}

TEST(ValidatingBsdfReader, TooFewElevationalSamples1) {
  BsdfData data(std::vector<float>({0.0f}), 1, 1);
  data.AddCoefficient(0, 0, 0, 1.0f);
  data.SetCdf(0, 0, 0, 0.0f);

  Flags flags{.is_bsdf = true, .uses_harmonic_extrapolation = false};

  std::string bsdf_file_bytes =
      MakeBsdfFile(flags, data, {}, {}, "", 1.0f, 1.0f, 1.0f);
  std::stringstream stream(bsdf_file_bytes);

  auto result = ReadFromStandardBsdf(stream);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error(),
            "The input must contain at least 3 elevational samples");
}

TEST(ValidatingBsdfReader, TooFewElevationalSamples2) {
  BsdfData data(std::vector<float>({0.0f, 1.0}), 1, 1);
  data.AddCoefficient(0, 0, 0, 1.0f);
  data.AddCoefficient(0, 0, 0, 1.0f);
  data.SetCdf(0, 0, 0, 0.0f);

  Flags flags{.is_bsdf = true, .uses_harmonic_extrapolation = false};

  std::string bsdf_file_bytes =
      MakeBsdfFile(flags, data, {}, {}, "", 1.0f, 1.0f, 1.0f);
  std::stringstream stream(bsdf_file_bytes);

  auto result = ReadFromStandardBsdf(stream);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error(),
            "The input must contain at least 3 elevational samples");
}

TEST(ValidatingBsdfReader, IgnoresExtraBasisFunctions) {
  BsdfData data(std::vector<float>({-1.0f, 0.0f, 1.0f}), 2, 1);
  data.AddCoefficient(0, 0, 0, 0, 1.0f);
  data.AddCoefficient(0, 0, 0, 1, 1.0f);
  data.AddCoefficient(0, 0, 0, 2, 1.0f);
  data.AddCoefficient(0, 0, 1, 0, 1.0f);
  data.AddCoefficient(0, 0, 1, 1, 1.0f);
  data.AddCoefficient(0, 0, 1, 2, 1.0f);
  data.AddCoefficient(0, 0, 2, 0, 1.0f);
  data.AddCoefficient(0, 0, 2, 1, 1.0f);
  data.AddCoefficient(0, 0, 2, 2, 1.0f);
  data.AddCoefficient(1, 0, 0, 0, 1.0f);
  data.AddCoefficient(1, 0, 0, 1, 1.0f);
  data.AddCoefficient(1, 0, 0, 2, 1.0f);
  data.AddCoefficient(1, 0, 1, 0, 1.0f);
  data.AddCoefficient(1, 0, 1, 1, 1.0f);
  data.AddCoefficient(1, 0, 1, 2, 1.0f);
  data.AddCoefficient(1, 0, 2, 0, 1.0f);
  data.AddCoefficient(1, 0, 2, 1, 1.0f);
  data.AddCoefficient(1, 0, 2, 2, 1.0f);
  data.SetCdf(0, 0, 0, 0.0f);
  data.SetCdf(0, 0, 1, 0.0f);
  data.SetCdf(0, 0, 2, 0.0f);
  data.SetCdf(0, 1, 0, 0.0f);
  data.SetCdf(0, 1, 1, 0.0f);
  data.SetCdf(0, 1, 2, 0.0f);
  data.SetCdf(0, 2, 0, 0.0f);
  data.SetCdf(0, 2, 1, 0.0f);
  data.SetCdf(0, 2, 2, 0.0f);
  data.SetCdf(1, 0, 0, 0.0f);
  data.SetCdf(1, 0, 1, 1.0f);
  data.SetCdf(1, 0, 2, 1.0f);
  data.SetCdf(1, 1, 0, 1.0f);
  data.SetCdf(1, 1, 1, 1.0f);
  data.SetCdf(1, 1, 2, 1.0f);
  data.SetCdf(1, 2, 0, 1.0f);
  data.SetCdf(1, 2, 1, 1.0f);
  data.SetCdf(1, 2, 2, 1.0f);

  Flags flags{.is_bsdf = true, .uses_harmonic_extrapolation = false};

  std::string bsdf_file_bytes =
      MakeBsdfFile(flags, data, {}, {}, "", 1.0f, 1.0f, 1.0f);
  std::stringstream stream(bsdf_file_bytes);

  auto result = ReadFromStandardBsdf(stream);
  ASSERT_TRUE(result);
  EXPECT_THAT(result->cdf,
              ElementsAre(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0));
}

TEST(ValidatingBsdfReader, OneColorChannel) {
  BsdfData data(std::vector<float>({-1.0f, 0.0f, 1.0f}), 1, 1);
  data.AddCoefficient(0, 0, 0, 1.0f);
  data.AddCoefficient(0, 1, 0, 2.0f);
  data.AddCoefficient(0, 2, 0, 3.0f);
  data.AddCoefficient(0, 0, 1, 4.0f);
  data.AddCoefficient(0, 1, 1, 5.0f);
  data.AddCoefficient(0, 2, 1, 6.0f);
  data.AddCoefficient(0, 0, 2, 7.0f);
  data.AddCoefficient(0, 1, 2, 8.0f);
  data.AddCoefficient(0, 2, 2, 9.0f);
  data.SetCdf(0, 0, 0, 0.0f);
  data.SetCdf(0, 0, 1, 0.0f);
  data.SetCdf(0, 0, 2, 0.0f);
  data.SetCdf(0, 1, 0, 0.0f);
  data.SetCdf(0, 1, 1, 0.0f);
  data.SetCdf(0, 1, 2, 0.0f);
  data.SetCdf(0, 2, 0, 0.0f);
  data.SetCdf(0, 2, 1, 0.0f);
  data.SetCdf(0, 2, 2, 0.0f);

  Flags flags{.is_bsdf = true, .uses_harmonic_extrapolation = false};

  std::string bsdf_file_bytes =
      MakeBsdfFile(flags, data, {}, {}, "", 1.0f, 2.0f, 3.0f);
  std::stringstream stream(bsdf_file_bytes);

  auto result = ReadFromStandardBsdf(stream);
  ASSERT_TRUE(result);
  EXPECT_THAT(result->y_coefficients,
              ElementsAre(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0));
  EXPECT_THAT(result->r_coefficients, IsEmpty());
  EXPECT_THAT(result->g_coefficients, IsEmpty());
  EXPECT_EQ(1.0, result->index_of_refraction);
  EXPECT_EQ(2.0, result->roughness_top);
  EXPECT_EQ(3.0, result->roughness_bottom);
}

TEST(ValidatingBsdfReader, ThreeColorChannel) {
  BsdfData data(std::vector<float>({-1.0f, 0.0f, 1.0f}), 1, 3);
  data.AddCoefficient(0, 0, 0, 1.0f);
  data.AddCoefficient(0, 1, 0, 2.0f);
  data.AddCoefficient(0, 2, 0, 3.0f);
  data.AddCoefficient(0, 0, 1, 4.0f);
  data.AddCoefficient(0, 1, 1, 5.0f);
  data.AddCoefficient(0, 2, 1, 6.0f);
  data.AddCoefficient(0, 0, 2, 7.0f);
  data.AddCoefficient(0, 1, 2, 8.0f);
  data.AddCoefficient(0, 2, 2, 9.0f);
  data.AddCoefficient(1, 0, 0, 2.0f);
  data.AddCoefficient(1, 1, 0, 2.0f);
  data.AddCoefficient(1, 2, 0, 2.0f);
  data.AddCoefficient(1, 0, 1, 2.0f);
  data.AddCoefficient(1, 1, 1, 2.0f);
  data.AddCoefficient(1, 2, 1, 2.0f);
  data.AddCoefficient(1, 0, 2, 2.0f);
  data.AddCoefficient(1, 1, 2, 2.0f);
  data.AddCoefficient(1, 2, 2, 2.0f);
  data.AddCoefficient(2, 0, 0, 3.0f);
  data.AddCoefficient(2, 1, 0, 3.0f);
  data.AddCoefficient(2, 2, 0, 3.0f);
  data.AddCoefficient(2, 0, 1, 3.0f);
  data.AddCoefficient(2, 1, 1, 3.0f);
  data.AddCoefficient(2, 2, 1, 3.0f);
  data.AddCoefficient(2, 0, 2, 3.0f);
  data.AddCoefficient(2, 1, 2, 3.0f);
  data.AddCoefficient(2, 2, 2, 3.0f);
  data.SetCdf(0, 0, 0, 0.0f);
  data.SetCdf(0, 0, 1, 0.0f);
  data.SetCdf(0, 0, 2, 0.0f);
  data.SetCdf(0, 1, 0, 0.0f);
  data.SetCdf(0, 1, 1, 0.0f);
  data.SetCdf(0, 1, 2, 0.0f);
  data.SetCdf(0, 2, 0, 0.0f);
  data.SetCdf(0, 2, 1, 0.0f);
  data.SetCdf(0, 2, 2, 0.0f);

  Flags flags{.is_bsdf = true, .uses_harmonic_extrapolation = false};

  std::string bsdf_file_bytes =
      MakeBsdfFile(flags, data, {}, {}, "", 1.0f, 2.0f, 3.0f);
  std::stringstream stream(bsdf_file_bytes);

  auto result = ReadFromStandardBsdf(stream);
  ASSERT_TRUE(result);
  EXPECT_THAT(result->y_coefficients,
              ElementsAre(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0));
  EXPECT_THAT(result->r_coefficients,
              ElementsAre(2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0));
  EXPECT_THAT(result->g_coefficients,
              ElementsAre(3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0));
  EXPECT_EQ(1.0, result->index_of_refraction);
  EXPECT_EQ(2.0, result->roughness_top);
  EXPECT_EQ(3.0, result->roughness_bottom);
}

}  // namespace
}  // namespace libfbsdf
