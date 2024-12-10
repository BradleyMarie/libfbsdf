#include "libfbsdf/readers/validating_bsdf_reader.h"

#include <expected>
#include <iostream>
#include <sstream>
#include <string>

#include "googlemock/include/gmock/gmock.h"
#include "googletest/include/gtest/gtest.h"
#include "libfbsdf/bsdf_reader.h"
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
using ::testing::_;
using ::testing::AtMost;
using ::testing::ElementsAre;
using ::testing::Return;

class StartFailsValidatingBsdfReader : public ValidatingBsdfReader {
 public:
  std::expected<Options, std::string> Start(
      const Flags& flags, uint32_t num_basis_functions,
      size_t num_color_channels, size_t longest_series_length,
      float index_of_refraction, float roughness_top, float roughness_bottom) {
    return std::unexpected("Start");
  }
};

TEST(ValidatingBsdfReader, StartFails) {
  std::stringstream stream(MakeEmptyBsdfFile(1.0f, 1.0f, 1.0f));
  auto result = StartFailsValidatingBsdfReader().ReadFrom(stream);
  ASSERT_FALSE(result);
  EXPECT_EQ("Start", result.error());
}

class MockValidatingBsdfReader : public ValidatingBsdfReader {
 public:
  std::expected<Options, std::string> Start(
      const Flags& flags, uint32_t num_basis_functions,
      size_t num_color_channels, size_t longest_series_length,
      float index_of_refraction, float roughness_top, float roughness_bottom) {
    return Options();
  }

  MOCK_METHOD(void, HandleElevationalSamples, (std::vector<float>), (override));
  MOCK_METHOD(void, HandleParameterSampleCounts, (std::vector<uint32_t>),
              (override));
  MOCK_METHOD(void, HandleParameterSamples, (std::vector<float>), (override));
  MOCK_METHOD(void, HandleCdf, (std::vector<float>), (override));
  MOCK_METHOD(void, HandleSeries,
              ((std::vector<std::pair<uint32_t, uint32_t>>)), (override));
  MOCK_METHOD(void, HandleCoefficients, (std::vector<float>), (override));
};

TEST(ValidatingBsdfReader, ParsesEmpty) {
  std::stringstream stream(MakeEmptyBsdfFile(1.0f, 1.0f, 1.0f));
  auto result = MockValidatingBsdfReader().ReadFrom(stream);
  EXPECT_TRUE(result);
}

TEST(ValidatingBsdfReader, ParsesMinimal) {
  std::stringstream stream(MakeMinimalBsdfFile(1.0f, 1.0f, 1.0f));
  MockValidatingBsdfReader mock_reader;

  EXPECT_CALL(mock_reader, HandleElevationalSamples(ElementsAre(1.0f)))
      .Times(1);
  EXPECT_CALL(mock_reader, HandleParameterSampleCounts(ElementsAre(1)))
      .Times(1);
  EXPECT_CALL(mock_reader, HandleParameterSamples(ElementsAre(1.0f))).Times(1);
  EXPECT_CALL(mock_reader, HandleCdf(ElementsAre(1.0f))).Times(1);
  EXPECT_CALL(mock_reader, HandleSeries(ElementsAre(std::make_pair(0, 1))))
      .Times(1);
  EXPECT_CALL(mock_reader, HandleCoefficients(ElementsAre(1.0f))).Times(1);

  auto result = mock_reader.ReadFrom(stream);
  EXPECT_TRUE(result);
}

TEST(ValidatingBsdfReader, BadlyOrderedElevationalSamples) {
  BsdfData data(std::vector<float>({0.75f, 0.25f}), 1, 1);
  data.AddCoefficient(0, 0, 0, std::numeric_limits<float>::quiet_NaN());
  data.SetCdf(0, 0, 0, std::numeric_limits<float>::quiet_NaN());

  Flags flags{.is_bsdf = true, .uses_harmonic_extrapolation = false};

  std::stringstream stream(
      MakeBsdfFile(flags, data, {}, {}, "", 1.0f, 1.0f, 1.0f));
  auto result = MockValidatingBsdfReader().ReadFrom(stream);
  ASSERT_FALSE(result);
  EXPECT_EQ("Input contained improperly ordered elevational samples",
            result.error());
}

TEST(ValidatingBsdfReader, TestDataLoads) {
  for (const auto& [file_name, file_params] : kTestDataFiles) {
    std::cout << file_name << std::endl;
    MockValidatingBsdfReader mock_reader;
    EXPECT_CALL(mock_reader, HandleElevationalSamples(_)).Times(AtMost(1));
    EXPECT_CALL(mock_reader, HandleParameterSampleCounts(_)).Times(AtMost(1));
    EXPECT_CALL(mock_reader, HandleParameterSamples(_)).Times(AtMost(1));
    EXPECT_CALL(mock_reader, HandleCdf(_)).Times(AtMost(1));
    EXPECT_CALL(mock_reader, HandleSeries(_)).Times(AtMost(1));
    EXPECT_CALL(mock_reader, HandleCoefficients(_)).Times(AtMost(1));
    EXPECT_TRUE(mock_reader.ReadFrom(*OpenTestData(file_name)));
  }
}

}  // namespace
}  // namespace libfbsdf