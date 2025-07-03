#include "libfbsdf/readers/validating_bsdf_reader.h"

#include <cstddef>
#include <cstdint>
#include <expected>
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
using ::testing::ElementsAre;
using ::testing::Return;
using ::testing::SizeIs;

class StartFailsValidatingBsdfReader : public ValidatingBsdfReader {
 public:
  std::expected<Options, std::string> Start(const Flags& flags,
                                            uint32_t num_basis_functions,
                                            size_t num_color_channels,
                                            float index_of_refraction,
                                            float roughness_top,
                                            float roughness_bottom) {
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
  MockValidatingBsdfReader(bool relaxed = true)
      : ValidatingBsdfReader({relaxed, relaxed, relaxed}) {}

  std::expected<Options, std::string> Start(const Flags& flags,
                                            uint32_t num_basis_functions,
                                            size_t num_color_channels,
                                            float index_of_refraction,
                                            float roughness_top,
                                            float roughness_bottom) {
    return Options();
  }

  MOCK_METHOD((std::expected<void, std::string>), HandleElevationalSamples,
              (std::vector<float>), (override));
  MOCK_METHOD((std::expected<void, std::string>), HandleParameterSampleCounts,
              (std::vector<uint32_t>), (override));
  MOCK_METHOD((std::expected<void, std::string>), HandleParameterSamples,
              (std::vector<float>), (override));
  MOCK_METHOD((std::expected<void, std::string>), HandleCdf,
              (std::vector<float>), (override));
  MOCK_METHOD((std::expected<void, std::string>), HandleSeries,
              ((std::vector<std::pair<uint32_t, uint32_t>>)), (override));
  MOCK_METHOD((std::expected<void, std::string>), HandleCoefficients,
              (std::vector<float>), (override));
};

TEST(ValidatingBsdfReader, ParsesEmpty) {
  std::stringstream stream(MakeEmptyBsdfFile(1.0f, 1.0f, 1.0f));
  auto result = MockValidatingBsdfReader().ReadFrom(stream);
  EXPECT_TRUE(result);
}

TEST(ValidatingBsdfReader, HandleElevationalSamplesFails) {
  std::stringstream stream(MakeMinimalBsdfFile(1.0f, 1.0f, 1.0f));
  MockValidatingBsdfReader mock_reader;

  EXPECT_CALL(mock_reader, HandleElevationalSamples(ElementsAre(1.0f)))
      .WillOnce(Return(std::unexpected("HandleElevationalSamples")));

  auto result = mock_reader.ReadFrom(stream);
  ASSERT_FALSE(result);
  EXPECT_EQ("HandleElevationalSamples", result.error());
}

TEST(ValidatingBsdfReader, HandleParameterSampleCountsFails) {
  std::stringstream stream(MakeMinimalBsdfFile(1.0f, 1.0f, 1.0f));
  MockValidatingBsdfReader mock_reader;

  EXPECT_CALL(mock_reader, HandleElevationalSamples(ElementsAre(1.0f)))
      .WillOnce(Return(std::expected<void, std::string>()));
  EXPECT_CALL(mock_reader, HandleParameterSampleCounts(ElementsAre(1)))
      .WillOnce(Return(std::unexpected("HandleParameterSampleCounts")));

  auto result = mock_reader.ReadFrom(stream);
  ASSERT_FALSE(result);
  EXPECT_EQ("HandleParameterSampleCounts", result.error());
}

TEST(ValidatingBsdfReader, HandleParameterSamplesFails) {
  std::stringstream stream(MakeMinimalBsdfFile(1.0f, 1.0f, 1.0f));
  MockValidatingBsdfReader mock_reader;

  EXPECT_CALL(mock_reader, HandleElevationalSamples(ElementsAre(1.0f)))
      .WillOnce(Return(std::expected<void, std::string>()));
  EXPECT_CALL(mock_reader, HandleParameterSampleCounts(ElementsAre(1)))
      .WillOnce(Return(std::expected<void, std::string>()));
  EXPECT_CALL(mock_reader, HandleParameterSamples(ElementsAre(1.0f)))
      .WillOnce(Return(std::unexpected("HandleParameterSamples")));

  auto result = mock_reader.ReadFrom(stream);
  ASSERT_FALSE(result);
  EXPECT_EQ("HandleParameterSamples", result.error());
}

TEST(ValidatingBsdfReader, HandleCdfFails) {
  std::stringstream stream(MakeMinimalBsdfFile(1.0f, 1.0f, 1.0f));
  MockValidatingBsdfReader mock_reader;

  EXPECT_CALL(mock_reader, HandleElevationalSamples(ElementsAre(1.0f)))
      .WillOnce(Return(std::expected<void, std::string>()));
  EXPECT_CALL(mock_reader, HandleParameterSampleCounts(ElementsAre(1)))
      .WillOnce(Return(std::expected<void, std::string>()));
  EXPECT_CALL(mock_reader, HandleParameterSamples(ElementsAre(1.0f)))
      .WillOnce(Return(std::expected<void, std::string>()));
  EXPECT_CALL(mock_reader, HandleCdf(ElementsAre(0.0f)))
      .WillOnce(Return(std::unexpected("HandleCdf")));

  auto result = mock_reader.ReadFrom(stream);
  ASSERT_FALSE(result);
  EXPECT_EQ("HandleCdf", result.error());
}

TEST(ValidatingBsdfReader, HandleSeriesFails) {
  std::stringstream stream(MakeMinimalBsdfFile(1.0f, 1.0f, 1.0f));
  MockValidatingBsdfReader mock_reader;

  EXPECT_CALL(mock_reader, HandleElevationalSamples(ElementsAre(1.0f)))
      .WillOnce(Return(std::expected<void, std::string>()));
  EXPECT_CALL(mock_reader, HandleParameterSampleCounts(ElementsAre(1)))
      .WillOnce(Return(std::expected<void, std::string>()));
  EXPECT_CALL(mock_reader, HandleParameterSamples(ElementsAre(1.0f)))
      .WillOnce(Return(std::expected<void, std::string>()));
  EXPECT_CALL(mock_reader, HandleCdf(ElementsAre(0.0f)))
      .WillOnce(Return(std::expected<void, std::string>()));
  EXPECT_CALL(mock_reader, HandleSeries(ElementsAre(std::make_pair(0, 1))))
      .WillOnce(Return(std::unexpected("HandleSeries")));

  auto result = mock_reader.ReadFrom(stream);
  ASSERT_FALSE(result);
  EXPECT_EQ("HandleSeries", result.error());
}

TEST(ValidatingBsdfReader, HandleCoefficientsFails) {
  std::stringstream stream(MakeMinimalBsdfFile(1.0f, 1.0f, 1.0f));
  MockValidatingBsdfReader mock_reader;

  EXPECT_CALL(mock_reader, HandleElevationalSamples(ElementsAre(1.0f)))
      .WillOnce(Return(std::expected<void, std::string>()));
  EXPECT_CALL(mock_reader, HandleParameterSampleCounts(ElementsAre(1)))
      .WillOnce(Return(std::expected<void, std::string>()));
  EXPECT_CALL(mock_reader, HandleParameterSamples(ElementsAre(1.0f)))
      .WillOnce(Return(std::expected<void, std::string>()));
  EXPECT_CALL(mock_reader, HandleCdf(ElementsAre(0.0f)))
      .WillOnce(Return(std::expected<void, std::string>()));
  EXPECT_CALL(mock_reader, HandleSeries(ElementsAre(std::make_pair(0, 1))))
      .WillOnce(Return(std::expected<void, std::string>()));
  EXPECT_CALL(mock_reader, HandleCoefficients(ElementsAre(1.0f)))
      .WillOnce(Return(std::unexpected("HandleCoefficients")));

  auto result = mock_reader.ReadFrom(stream);
  ASSERT_FALSE(result);
  EXPECT_EQ("HandleCoefficients", result.error());
}

TEST(ValidatingBsdfReader, ParsesMinimal) {
  std::stringstream stream(MakeMinimalBsdfFile(1.0f, 1.0f, 1.0f));
  MockValidatingBsdfReader mock_reader;

  EXPECT_CALL(mock_reader, HandleElevationalSamples(ElementsAre(1.0f)))
      .WillOnce(Return(std::expected<void, std::string>()));
  EXPECT_CALL(mock_reader, HandleParameterSampleCounts(ElementsAre(1)))
      .WillOnce(Return(std::expected<void, std::string>()));
  EXPECT_CALL(mock_reader, HandleParameterSamples(ElementsAre(1.0f)))
      .WillOnce(Return(std::expected<void, std::string>()));
  EXPECT_CALL(mock_reader, HandleCdf(ElementsAre(0.0f)))
      .WillOnce(Return(std::expected<void, std::string>()));
  EXPECT_CALL(mock_reader, HandleSeries(ElementsAre(std::make_pair(0, 1))))
      .WillOnce(Return(std::expected<void, std::string>()));
  EXPECT_CALL(mock_reader, HandleCoefficients(ElementsAre(1.0f)))
      .WillOnce(Return(std::expected<void, std::string>()));

  auto result = mock_reader.ReadFrom(stream);
  EXPECT_TRUE(result);
}

TEST(ValidatingBsdfReader, TooLowElevationalSamples) {
  BsdfData data(std::vector<float>({-1.5}), 1, 1);
  data.AddCoefficient(0, 0, 0, std::numeric_limits<float>::quiet_NaN());
  data.SetCdf(0, 0, 0, std::numeric_limits<float>::quiet_NaN());

  Flags flags{.is_bsdf = true, .uses_harmonic_extrapolation = false};

  std::stringstream stream(
      MakeBsdfFile(flags, data, {}, {}, "", 1.0f, 1.0f, 1.0f));
  auto result = MockValidatingBsdfReader().ReadFrom(stream);
  ASSERT_FALSE(result);
  EXPECT_EQ("Input contained elevational samples that were out of range",
            result.error());
}

TEST(ValidatingBsdfReader, TooHighElevationalSamples) {
  BsdfData data(std::vector<float>({1.5}), 1, 1);
  data.AddCoefficient(0, 0, 0, std::numeric_limits<float>::quiet_NaN());
  data.SetCdf(0, 0, 0, std::numeric_limits<float>::quiet_NaN());

  Flags flags{.is_bsdf = true, .uses_harmonic_extrapolation = false};

  std::stringstream stream(
      MakeBsdfFile(flags, data, {}, {}, "", 1.0f, 1.0f, 1.0f));
  auto result = MockValidatingBsdfReader().ReadFrom(stream);
  ASSERT_FALSE(result);
  EXPECT_EQ("Input contained elevational samples that were out of range",
            result.error());
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

TEST(ValidatingBsdfReader, InvalidStartingCdf) {
  BsdfData data(std::vector<float>({0.0f}), 1, 1);
  data.AddCoefficient(0, 0, 0, 1.0f);
  data.SetCdf(0, 0, 0, 0.1f);

  Flags flags{.is_bsdf = true, .uses_harmonic_extrapolation = false};

  std::stringstream stream(
      MakeBsdfFile(flags, data, {}, {}, "", 1.0f, 1.0f, 1.0f));
  MockValidatingBsdfReader mock_reader(false);

  EXPECT_CALL(mock_reader, HandleElevationalSamples(ElementsAre(0.0f)))
      .WillOnce(Return(std::expected<void, std::string>()));

  auto result = mock_reader.ReadFrom(stream);
  ASSERT_FALSE(result);
  EXPECT_EQ("Input contained a CDF range that did not start with zero",
            result.error());
}

TEST(ValidatingBsdfReader, TooLowCdf) {
  BsdfData data(std::vector<float>({0.0f}), 1, 1);
  data.AddCoefficient(0, 0, 0, 1.0f);
  data.SetCdf(0, 0, 0, -0.5f);

  Flags flags{.is_bsdf = true, .uses_harmonic_extrapolation = false};

  std::stringstream stream(
      MakeBsdfFile(flags, data, {}, {}, "", 1.0f, 1.0f, 1.0f));
  MockValidatingBsdfReader mock_reader(false);

  EXPECT_CALL(mock_reader, HandleElevationalSamples(ElementsAre(0.0f)))
      .WillOnce(Return(std::expected<void, std::string>()));

  auto result = mock_reader.ReadFrom(stream);
  ASSERT_FALSE(result);
  EXPECT_EQ("Input contained a CDF value that was out of range",
            result.error());
}

TEST(ValidatingBsdfReader, TooHighCdf) {
  BsdfData data(std::vector<float>({0.0f}), 1, 1);
  data.AddCoefficient(0, 0, 0, 1.0f);
  data.SetCdf(0, 0, 0, 1.5f);

  Flags flags{.is_bsdf = true, .uses_harmonic_extrapolation = false};

  std::stringstream stream(
      MakeBsdfFile(flags, data, {}, {}, "", 1.0f, 1.0f, 1.0f));
  MockValidatingBsdfReader mock_reader(false);

  EXPECT_CALL(mock_reader, HandleElevationalSamples(ElementsAre(0.0f)))
      .WillOnce(Return(std::expected<void, std::string>()));

  auto result = mock_reader.ReadFrom(stream);
  ASSERT_FALSE(result);
  EXPECT_EQ("Input contained a CDF value that was out of range",
            result.error());
}

TEST(ValidatingBsdfReader, ClampedLowCdf) {
  BsdfData data(std::vector<float>({0.0f}), 1, 1);
  data.AddCoefficient(0, 0, 0, 1.0f);
  data.SetCdf(0, 0, 0, -0.5f);

  Flags flags{.is_bsdf = true, .uses_harmonic_extrapolation = false};

  std::stringstream stream(
      MakeBsdfFile(flags, data, {}, {}, "", 1.0f, 1.0f, 1.0f));
  MockValidatingBsdfReader mock_reader;

  EXPECT_CALL(mock_reader, HandleElevationalSamples(ElementsAre(0.0f)))
      .WillOnce(Return(std::expected<void, std::string>()));
  EXPECT_CALL(mock_reader, HandleCdf(ElementsAre(0.0f)))
      .WillOnce(Return(std::expected<void, std::string>()));
  EXPECT_CALL(mock_reader, HandleSeries(ElementsAre(std::make_pair(0, 1))))
      .WillOnce(Return(std::expected<void, std::string>()));
  EXPECT_CALL(mock_reader, HandleCoefficients(ElementsAre(1.0f)))
      .WillOnce(Return(std::expected<void, std::string>()));

  auto result = mock_reader.ReadFrom(stream);
  EXPECT_TRUE(result);
}

TEST(ValidatingBsdfReader, ClampedHighCdf) {
  BsdfData data(std::vector<float>({0.0, 0.5}), 1, 1);
  data.AddCoefficient(0, 0, 0, 1.0f);
  data.SetCdf(0, 0, 0, 0.0f);
  data.SetCdf(0, 1, 0, 1.5f);
  data.SetCdf(0, 0, 1, 1.0f);
  data.SetCdf(0, 1, 1, 1.0f);

  Flags flags{.is_bsdf = true, .uses_harmonic_extrapolation = false};

  std::stringstream stream(
      MakeBsdfFile(flags, data, {}, {}, "", 1.0f, 1.0f, 1.0f));
  MockValidatingBsdfReader mock_reader;

  EXPECT_CALL(mock_reader, HandleElevationalSamples(ElementsAre(0.0f, 0.5f)))
      .WillOnce(Return(std::expected<void, std::string>()));
  EXPECT_CALL(mock_reader, HandleCdf(ElementsAre(0.0f, 1.0f, 1.0f, 1.0f)))
      .WillOnce(Return(std::expected<void, std::string>()));
  EXPECT_CALL(mock_reader, HandleSeries(ElementsAre(
                               std::make_pair(0, 1), std::make_pair(1, 0),
                               std::make_pair(1, 0), std::make_pair(1, 0))))
      .WillOnce(Return(std::expected<void, std::string>()));
  EXPECT_CALL(mock_reader, HandleCoefficients(ElementsAre(1.0f)))
      .WillOnce(Return(std::expected<void, std::string>()));

  auto result = mock_reader.ReadFrom(stream);
  EXPECT_TRUE(result);
}

TEST(ValidatingBsdfReader, TooLowLongestLength) {
  BsdfData data(std::vector<float>({0.0f}), 1, 1);
  data.AddCoefficient(0, 0, 0, 1.0f);
  data.SetCdf(0, 0, 0, 0.0f);

  Flags flags{.is_bsdf = true, .uses_harmonic_extrapolation = false};

  std::string bsdf_file_bytes =
      MakeBsdfFile(flags, data, {}, {}, "", 1.0f, 1.0f, 1.0f);
  bsdf_file_bytes[20] = '\0';

  std::stringstream stream(bsdf_file_bytes);
  MockValidatingBsdfReader mock_reader(false);

  EXPECT_CALL(mock_reader, HandleElevationalSamples(ElementsAre(0.0f)))
      .WillOnce(Return(std::expected<void, std::string>()));
  EXPECT_CALL(mock_reader, HandleCdf(ElementsAre(0.0f)))
      .WillOnce(Return(std::expected<void, std::string>()));

  auto result = mock_reader.ReadFrom(stream);
  ASSERT_FALSE(result);
  EXPECT_EQ(
      "Input contained a series that was longer than the maximum length "
      "defined in the input",
      result.error());
}

TEST(ValidatingBsdfReader, TooHighOffset) {
  BsdfData data(std::vector<float>({0.0f}), 1, 1);
  data.AddCoefficient(0, 0, 0, 1.0f);
  data.SetCdf(0, 0, 0, 0.0f);

  Flags flags{.is_bsdf = true, .uses_harmonic_extrapolation = false};

  std::string bsdf_file_bytes =
      MakeBsdfFile(flags, data, {}, {}, "", 1.0f, 1.0f, 1.0f);
  bsdf_file_bytes[74] = 16;

  std::stringstream stream(bsdf_file_bytes);
  MockValidatingBsdfReader mock_reader;

  EXPECT_CALL(mock_reader, HandleElevationalSamples(ElementsAre(0.0f)))
      .WillOnce(Return(std::expected<void, std::string>()));
  EXPECT_CALL(mock_reader, HandleCdf(ElementsAre(0.0f)))
      .WillOnce(Return(std::expected<void, std::string>()));

  auto result = mock_reader.ReadFrom(stream);
  ASSERT_FALSE(result);
  EXPECT_EQ("Input contained an offset that was out of bounds", result.error());
}

TEST(ValidatingBsdfReader, TooLongLength) {
  BsdfData data(std::vector<float>({0.0f}), 1, 1);
  data.AddCoefficient(0, 0, 0, 1.0f);
  data.SetCdf(0, 0, 0, 0.0f);

  Flags flags{.is_bsdf = true, .uses_harmonic_extrapolation = false};

  std::string bsdf_file_bytes =
      MakeBsdfFile(flags, data, {}, {}, "", 1.0f, 1.0f, 1.0f);
  bsdf_file_bytes[78] = 16;

  std::stringstream stream(bsdf_file_bytes);
  MockValidatingBsdfReader mock_reader;

  EXPECT_CALL(mock_reader, HandleElevationalSamples(ElementsAre(0.0f)))
      .WillOnce(Return(std::expected<void, std::string>()));
  EXPECT_CALL(mock_reader, HandleCdf(ElementsAre(0.0f)))
      .WillOnce(Return(std::expected<void, std::string>()));

  auto result = mock_reader.ReadFrom(stream);
  ASSERT_FALSE(result);
  EXPECT_EQ("Input contained a series that extended out of bounds",
            result.error());
}

TEST(ValidatingBsdfReader, TestDataLoads) {
  for (const auto& [file_name, file_params] : kTestDataFiles) {
    MockValidatingBsdfReader mock_reader;

    if (file_params.num_elevational_samples != 0) {
      EXPECT_CALL(mock_reader, HandleElevationalSamples(
                                   SizeIs(file_params.num_elevational_samples)))
          .WillOnce(Return(std::expected<void, std::string>()));
    }

    if (file_params.num_parameters != 0) {
      EXPECT_CALL(mock_reader, HandleParameterSampleCounts(
                                   SizeIs(file_params.num_parameters)))
          .WillOnce(Return(std::expected<void, std::string>()));
    }

    if (file_params.num_parameter_values != 0) {
      EXPECT_CALL(mock_reader, HandleParameterSamples(
                                   SizeIs(file_params.num_parameter_values)))
          .WillOnce(Return(std::expected<void, std::string>()));
    }

    if (file_params.num_elevational_samples != 0 &&
        file_params.num_basis_functions != 0) {
      EXPECT_CALL(mock_reader,
                  HandleCdf(SizeIs(file_params.num_elevational_samples *
                                   file_params.num_elevational_samples *
                                   file_params.num_basis_functions)))
          .WillOnce(Return(std::expected<void, std::string>()));
    }

    if (file_params.num_elevational_samples != 0) {
      EXPECT_CALL(mock_reader,
                  HandleSeries(SizeIs(file_params.num_elevational_samples *
                                      file_params.num_elevational_samples)))
          .WillOnce(Return(std::expected<void, std::string>()));
    }

    if (file_params.num_coefficients != 0) {
      EXPECT_CALL(mock_reader,
                  HandleCoefficients(SizeIs(file_params.num_coefficients)))
          .WillOnce(Return(std::expected<void, std::string>()));
    }

    EXPECT_TRUE(mock_reader.ReadFrom(*OpenTestData(file_name)));
  }
}

}  // namespace
}  // namespace libfbsdf
