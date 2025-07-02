#include "libfbsdf/bsdf_reader.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <istream>
#include <memory>
#include <sstream>
#include <string>

#include "googlemock/include/gmock/gmock.h"
#include "googletest/include/gtest/gtest.h"
#include "libfbsdf/test_bsdf_writer.h"
#include "test_data/test_data.h"

namespace libfbsdf {
namespace {

using ::libfbsdf::testing::FileParams;
using ::libfbsdf::testing::kTestDataFiles;
using ::libfbsdf::testing::MakeEmptyBsdfFile;
using ::libfbsdf::testing::MakeMinimalBsdfFile;
using ::libfbsdf::testing::MakeNonFiniteBsdfFile;
using ::libfbsdf::testing::OpenTestData;
using ::testing::_;
using ::testing::Return;

class FailImmediatelyBsdfReader : public BsdfReader {
 public:
  std::expected<Options, std::string> Start(
      const Flags& flags, size_t num_elevational_samples,
      size_t num_basis_functions, size_t num_coefficients,
      size_t num_color_channels, size_t longest_series_length,
      size_t num_parameters, size_t num_parameter_values,
      size_t metadata_size_bytes, float index_of_refraction,
      float roughness_top, float roughness_bottom) {
    return std::unexpected("Start");
  }
};

TEST(BsdfReader, StartFails) {
  std::stringstream stream(MakeEmptyBsdfFile(1.0f, 1.0f, 1.0f));
  auto result = FailImmediatelyBsdfReader().ReadFrom(stream);
  ASSERT_FALSE(result);
  EXPECT_EQ("Start", result.error());
}

class MockBsdfReader : public BsdfReader {
 public:
  std::expected<Options, std::string> Start(
      const Flags& flags, size_t num_elevational_samples,
      size_t num_basis_functions, size_t num_coefficients,
      size_t num_color_channels, size_t longest_series_length,
      size_t num_parameters, size_t num_parameter_values,
      size_t metadata_size_bytes, float index_of_refraction,
      float roughness_top, float roughness_bottom) {
    return Options();
  }

  MOCK_METHOD((std::expected<void, std::string>), HandleElevationalSample,
              (float), (override));
  MOCK_METHOD((std::expected<void, std::string>), HandleSampleCount, (uint32_t),
              (override));
  MOCK_METHOD((std::expected<void, std::string>), HandleSamplePosition, (float),
              (override));
  MOCK_METHOD((std::expected<void, std::string>), HandleCdf, (float),
              (override));
  MOCK_METHOD((std::expected<void, std::string>), HandleSeries,
              (uint32_t, uint32_t), (override));
  MOCK_METHOD((std::expected<void, std::string>), HandleCoefficient, (float),
              (override));
  MOCK_METHOD((std::expected<void, std::string>), HandleMetadata, (std::string),
              (override));
};

TEST(BsdfReader, HandleElevationalSampleFails) {
  std::stringstream stream(MakeMinimalBsdfFile(1.0f, 1.0f, 1.0f));

  MockBsdfReader bsdf_reader;
  EXPECT_CALL(bsdf_reader, HandleElevationalSample(_))
      .WillOnce(Return(std::unexpected("HandleElevationalSample")));

  auto result = bsdf_reader.ReadFrom(stream);
  ASSERT_FALSE(result);
  EXPECT_EQ("HandleElevationalSample", result.error());
}

TEST(BsdfReader, HandleSampleCountFails) {
  std::stringstream stream(MakeMinimalBsdfFile(1.0f, 1.0f, 1.0f));

  MockBsdfReader bsdf_reader;
  EXPECT_CALL(bsdf_reader, HandleElevationalSample(_))
      .WillRepeatedly(Return(std::expected<void, std::string>()));
  EXPECT_CALL(bsdf_reader, HandleSampleCount(_))
      .WillRepeatedly(Return(std::unexpected("HandleSampleCount")));

  auto result = bsdf_reader.ReadFrom(stream);
  ASSERT_FALSE(result);
  EXPECT_EQ("HandleSampleCount", result.error());
}

TEST(BsdfReader, HandleSamplePositionFails) {
  std::stringstream stream(MakeMinimalBsdfFile(1.0f, 1.0f, 1.0f));

  MockBsdfReader bsdf_reader;
  EXPECT_CALL(bsdf_reader, HandleElevationalSample(_))
      .WillRepeatedly(Return(std::expected<void, std::string>()));
  EXPECT_CALL(bsdf_reader, HandleSampleCount(_))
      .WillRepeatedly(Return(std::expected<void, std::string>()));
  EXPECT_CALL(bsdf_reader, HandleSamplePosition(_))
      .WillRepeatedly(Return(std::unexpected("HandleSamplePosition")));

  auto result = bsdf_reader.ReadFrom(stream);
  ASSERT_FALSE(result);
  EXPECT_EQ("HandleSamplePosition", result.error());
}

TEST(BsdfReader, HandleCdfFails) {
  std::stringstream stream(MakeMinimalBsdfFile(1.0f, 1.0f, 1.0f));

  MockBsdfReader bsdf_reader;
  EXPECT_CALL(bsdf_reader, HandleElevationalSample(_))
      .WillRepeatedly(Return(std::expected<void, std::string>()));
  EXPECT_CALL(bsdf_reader, HandleSampleCount(_))
      .WillRepeatedly(Return(std::expected<void, std::string>()));
  EXPECT_CALL(bsdf_reader, HandleSamplePosition(_))
      .WillRepeatedly(Return(std::expected<void, std::string>()));
  EXPECT_CALL(bsdf_reader, HandleCdf(_))
      .WillRepeatedly(Return(std::unexpected("HandleCdf")));

  auto result = bsdf_reader.ReadFrom(stream);
  ASSERT_FALSE(result);
  EXPECT_EQ("HandleCdf", result.error());
}

TEST(BsdfReader, HandleSeriesFails) {
  std::stringstream stream(MakeMinimalBsdfFile(1.0f, 1.0f, 1.0f));

  MockBsdfReader bsdf_reader;
  EXPECT_CALL(bsdf_reader, HandleElevationalSample(_))
      .WillRepeatedly(Return(std::expected<void, std::string>()));
  EXPECT_CALL(bsdf_reader, HandleSampleCount(_))
      .WillRepeatedly(Return(std::expected<void, std::string>()));
  EXPECT_CALL(bsdf_reader, HandleSamplePosition(_))
      .WillRepeatedly(Return(std::expected<void, std::string>()));
  EXPECT_CALL(bsdf_reader, HandleCdf(_))
      .WillRepeatedly(Return(std::expected<void, std::string>()));
  EXPECT_CALL(bsdf_reader, HandleSeries(_, _))
      .WillRepeatedly(Return(std::unexpected("HandleSeries")));

  auto result = bsdf_reader.ReadFrom(stream);
  ASSERT_FALSE(result);
  EXPECT_EQ("HandleSeries", result.error());
}

TEST(BsdfReader, HandleCoefficientFails) {
  std::stringstream stream(MakeMinimalBsdfFile(1.0f, 1.0f, 1.0f));

  MockBsdfReader bsdf_reader;
  EXPECT_CALL(bsdf_reader, HandleElevationalSample(_))
      .WillRepeatedly(Return(std::expected<void, std::string>()));
  EXPECT_CALL(bsdf_reader, HandleSampleCount(_))
      .WillRepeatedly(Return(std::expected<void, std::string>()));
  EXPECT_CALL(bsdf_reader, HandleSamplePosition(_))
      .WillRepeatedly(Return(std::expected<void, std::string>()));
  EXPECT_CALL(bsdf_reader, HandleCdf(_))
      .WillRepeatedly(Return(std::expected<void, std::string>()));
  EXPECT_CALL(bsdf_reader, HandleSeries(_, _))
      .WillRepeatedly(Return(std::expected<void, std::string>()));
  EXPECT_CALL(bsdf_reader, HandleCoefficient(_))
      .WillRepeatedly(Return(std::unexpected("HandleCoefficient")));

  auto result = bsdf_reader.ReadFrom(stream);
  ASSERT_FALSE(result);
  EXPECT_EQ("HandleCoefficient", result.error());
}

TEST(BsdfReader, HandleMetadataFails) {
  std::stringstream stream(MakeMinimalBsdfFile(1.0f, 1.0f, 1.0f));

  MockBsdfReader bsdf_reader;
  EXPECT_CALL(bsdf_reader, HandleElevationalSample(_))
      .WillRepeatedly(Return(std::expected<void, std::string>()));
  EXPECT_CALL(bsdf_reader, HandleSampleCount(_))
      .WillRepeatedly(Return(std::expected<void, std::string>()));
  EXPECT_CALL(bsdf_reader, HandleSamplePosition(_))
      .WillRepeatedly(Return(std::expected<void, std::string>()));
  EXPECT_CALL(bsdf_reader, HandleCdf(_))
      .WillRepeatedly(Return(std::expected<void, std::string>()));
  EXPECT_CALL(bsdf_reader, HandleSeries(_, _))
      .WillRepeatedly(Return(std::expected<void, std::string>()));
  EXPECT_CALL(bsdf_reader, HandleCoefficient(_))
      .WillRepeatedly(Return(std::expected<void, std::string>()));
  EXPECT_CALL(bsdf_reader, HandleMetadata(_))
      .WillRepeatedly(Return(std::unexpected("HandleMetadata")));

  auto result = bsdf_reader.ReadFrom(stream);
  ASSERT_FALSE(result);
  EXPECT_EQ("HandleMetadata", result.error());
}

class TestBsdfReader : public BsdfReader {
 public:
  TestBsdfReader(bool is_bsdf, bool uses_harmonic_extrapolation,
                 uint32_t num_elevational_samples, uint32_t num_basis_functions,
                 uint32_t num_coefficients, uint32_t num_color_channels,
                 uint32_t longest_series_length, uint32_t num_parameters,
                 uint32_t num_parameter_values, uint32_t metadata_size_bytes,
                 float index_of_refraction, float roughness_top,
                 float roughness_bottom,
                 const std::array<bool, 7>& parsed_parameters)
      : is_bsdf_(is_bsdf),
        uses_harmonic_extrapolation_(uses_harmonic_extrapolation),
        num_elevational_samples_(num_elevational_samples),
        num_basis_functions_(num_basis_functions),
        num_coefficients_(num_coefficients),
        num_color_channels_(num_color_channels),
        longest_series_length_(longest_series_length),
        num_parameters_(num_parameters),
        num_parameter_values_(num_parameter_values),
        metadata_size_bytes_(metadata_size_bytes),
        index_of_refraction_(index_of_refraction),
        roughness_top_(roughness_top),
        roughness_bottom_(roughness_bottom),
        parsed_parameters_(parsed_parameters) {}

  TestBsdfReader(const FileParams& file_params,
                 const std::array<bool, 7>& parsed_parameters)
      : is_bsdf_(file_params.is_bsdf),
        uses_harmonic_extrapolation_(file_params.uses_harmonic_extrapolation),
        num_elevational_samples_(file_params.num_elevational_samples),
        num_basis_functions_(file_params.num_basis_functions),
        num_coefficients_(file_params.num_coefficients),
        num_color_channels_(file_params.num_color_channels),
        longest_series_length_(file_params.longest_series_length),
        num_parameters_(file_params.num_parameters),
        num_parameter_values_(file_params.num_parameter_values),
        metadata_size_bytes_(file_params.metadata_size_bytes),
        index_of_refraction_(file_params.index_of_refraction),
        roughness_top_(file_params.roughness_top),
        roughness_bottom_(file_params.roughness_bottom),
        parsed_parameters_(parsed_parameters) {}

  std::expected<Options, std::string> Start(
      const Flags& flags, size_t num_elevational_samples,
      size_t num_basis_functions, size_t num_coefficients,
      size_t num_color_channels, size_t longest_series_length,
      size_t num_parameters, size_t num_parameter_values,
      size_t metadata_size_bytes, float index_of_refraction,
      float roughness_top, float roughness_bottom) {
    EXPECT_EQ(is_bsdf_, flags.is_bsdf);
    EXPECT_EQ(uses_harmonic_extrapolation_, flags.uses_harmonic_extrapolation);
    EXPECT_EQ(num_elevational_samples_, num_elevational_samples);
    EXPECT_EQ(num_basis_functions_, num_basis_functions);
    EXPECT_EQ(num_coefficients_, num_coefficients);
    EXPECT_EQ(num_color_channels_, num_color_channels);
    EXPECT_EQ(longest_series_length_, longest_series_length);
    EXPECT_EQ(num_parameters_, num_parameters);
    EXPECT_EQ(num_parameter_values_, num_parameter_values);
    EXPECT_EQ(metadata_size_bytes_, metadata_size_bytes);
    EXPECT_EQ(index_of_refraction_, index_of_refraction);
    EXPECT_EQ(roughness_top_, roughness_top);
    EXPECT_EQ(roughness_bottom_, roughness_bottom);

    Options options{
        .parse_elevational_samples = parsed_parameters_[0],
        .parse_parameter_sample_counts = parsed_parameters_[1],
        .parse_parameter_values = parsed_parameters_[2],
        .parse_cdf_mu = parsed_parameters_[3],
        .parse_series = parsed_parameters_[4],
        .parse_coefficients = parsed_parameters_[5],
        .parse_metadata = parsed_parameters_[6],
    };

    return options;
  }

  MOCK_METHOD((std::expected<void, std::string>), HandleElevationalSample,
              (float), (override));
  MOCK_METHOD((std::expected<void, std::string>), HandleSampleCount, (uint32_t),
              (override));
  MOCK_METHOD((std::expected<void, std::string>), HandleSamplePosition, (float),
              (override));
  MOCK_METHOD((std::expected<void, std::string>), HandleCdf, (float),
              (override));
  MOCK_METHOD((std::expected<void, std::string>), HandleSeries,
              (uint32_t, uint32_t), (override));
  MOCK_METHOD((std::expected<void, std::string>), HandleCoefficient, (float),
              (override));
  MOCK_METHOD((std::expected<void, std::string>), HandleMetadata, (std::string),
              (override));

 private:
  bool is_bsdf_;
  bool uses_harmonic_extrapolation_;
  uint32_t num_elevational_samples_;
  uint32_t num_basis_functions_;
  uint32_t num_coefficients_;
  uint32_t num_color_channels_;
  uint32_t longest_series_length_;
  uint32_t num_parameters_;
  uint32_t num_parameter_values_;
  uint32_t metadata_size_bytes_;
  float index_of_refraction_;
  float roughness_top_;
  float roughness_bottom_;
  std::array<bool, 7> parsed_parameters_;
};

TEST(BsdfReader, TestDataLoads) {
  for (const auto& [file_name, file_params] : kTestDataFiles) {
    for (size_t i = 0; i < 7; i++) {
      std::array<bool, 7> parsed_parameters = {i == 0, i == 1, i == 2, i == 3,
                                               i == 4, i == 5, i == 6};
      TestBsdfReader test_reader(file_params, parsed_parameters);

      if (parsed_parameters[0] && file_params.num_elevational_samples != 0) {
        EXPECT_CALL(test_reader, HandleElevationalSample(_))
            .Times(file_params.num_elevational_samples)
            .WillRepeatedly(Return(std::expected<void, std::string>()));
      }

      if (parsed_parameters[1] && file_params.num_parameters != 0) {
        EXPECT_CALL(test_reader, HandleSampleCount(_))
            .Times(file_params.num_parameters)
            .WillRepeatedly(Return(std::expected<void, std::string>()));
      }

      if (parsed_parameters[2] && file_params.num_parameter_values != 0) {
        EXPECT_CALL(test_reader, HandleSamplePosition(_))
            .Times(file_params.num_parameter_values)
            .WillRepeatedly(Return(std::expected<void, std::string>()));
      }

      if (parsed_parameters[3] && file_params.num_basis_functions != 0 &&
          file_params.num_elevational_samples != 0) {
        EXPECT_CALL(test_reader, HandleCdf(_))
            .Times(file_params.num_basis_functions *
                   file_params.num_elevational_samples *
                   file_params.num_elevational_samples)
            .WillRepeatedly(Return(std::expected<void, std::string>()));
      }

      if (parsed_parameters[4] && file_params.num_elevational_samples != 0) {
        EXPECT_CALL(test_reader, HandleSeries(_, _))
            .Times(file_params.num_elevational_samples *
                   file_params.num_elevational_samples)
            .WillRepeatedly(Return(std::expected<void, std::string>()));
      }

      if (parsed_parameters[5] && file_params.num_coefficients != 0) {
        EXPECT_CALL(test_reader, HandleCoefficient(_))
            .Times(file_params.num_coefficients)
            .WillRepeatedly(Return(std::expected<void, std::string>()));
      }

      if (parsed_parameters[6] && file_params.metadata_size_bytes != 0) {
        EXPECT_CALL(test_reader, HandleMetadata(_))
            .WillOnce(Return(std::expected<void, std::string>()));
      }

      EXPECT_TRUE(test_reader.ReadFrom(*OpenTestData(file_name)));
    }
  }
}

TEST(BsdfReader, ParsesEmptyBsdf) {
  for (size_t i = 0; i < 7; i++) {
    std::array<bool, 7> parsed_parameters = {i == 0, i == 1, i == 2, i == 3,
                                             i == 4, i == 5, i == 6};
    TestBsdfReader test_reader(
        /*is_bsdf=*/true,
        /*uses_harmonic_extrapolation=*/false,
        /*num_elevational_samples=*/0,
        /*num_basis_functions=*/0,
        /*num_coefficients=*/0,
        /*num_color_channels=*/0,
        /*longest_series_length=*/0,
        /*num_parameters=*/0,
        /*num_parameter_values=*/0,
        /*metadata_size_bytes=*/0,
        /*index_of_refraction=*/1.0f,
        /*roughness_top=*/1.0f,
        /*roughness_bottom=*/1.0f, parsed_parameters);

    std::stringstream stream(MakeEmptyBsdfFile(1.0f, 1.0f, 1.0f));
    EXPECT_TRUE(test_reader.ReadFrom(stream));
  }
}

TEST(BsdfReader, ParsesMinimalBsdf) {
  for (size_t i = 0; i < 7; i++) {
    std::array<bool, 7> parsed_parameters = {i == 0, i == 1, i == 2, i == 3,
                                             i == 4, i == 5, i == 6};
    TestBsdfReader test_reader(
        /*is_bsdf=*/true,
        /*uses_harmonic_extrapolation=*/false,
        /*num_elevational_samples=*/1,
        /*num_basis_functions=*/1,
        /*num_coefficients=*/1,
        /*num_color_channels=*/1,
        /*longest_series_length=*/1,
        /*num_parameters=*/1,
        /*num_parameter_values=*/1,
        /*metadata_size_bytes=*/4,
        /*index_of_refraction=*/1.0f,
        /*roughness_top=*/1.0f,
        /*roughness_bottom=*/1.0f, parsed_parameters);

    if (parsed_parameters[0]) {
      EXPECT_CALL(test_reader, HandleElevationalSample(1.0f))
          .WillOnce(Return(std::expected<void, std::string>()));
    }

    if (parsed_parameters[1]) {
      EXPECT_CALL(test_reader, HandleSampleCount(1))
          .WillOnce(Return(std::expected<void, std::string>()));
    }

    if (parsed_parameters[2]) {
      EXPECT_CALL(test_reader, HandleSamplePosition(1.0f))
          .WillOnce(Return(std::expected<void, std::string>()));
    }

    if (parsed_parameters[3]) {
      EXPECT_CALL(test_reader, HandleCdf(0.0f))
          .WillOnce(Return(std::expected<void, std::string>()));
    }

    if (parsed_parameters[4]) {
      EXPECT_CALL(test_reader, HandleSeries(0, 1))
          .WillOnce(Return(std::expected<void, std::string>()));
    }

    if (parsed_parameters[5]) {
      EXPECT_CALL(test_reader, HandleCoefficient(1.0f))
          .WillOnce(Return(std::expected<void, std::string>()));
    }

    if (parsed_parameters[6]) {
      EXPECT_CALL(test_reader, HandleMetadata("meta"))
          .WillOnce(Return(std::expected<void, std::string>()));
    }

    std::stringstream stream(MakeMinimalBsdfFile(1.0f, 1.0f, 1.0f));
    EXPECT_TRUE(test_reader.ReadFrom(stream));
  }
}

TEST(BsdfReader, TruncatedMinimalBsdfFails) {
  std::string minimal_bsdf = MakeMinimalBsdfFile(1.0f, 1.0f, 1.0f);
  for (size_t i = 0; i < 7; i++) {
    for (size_t i = 64; i < minimal_bsdf.size() - 1; i++) {
      std::array<bool, 7> parsed_parameters = {i == 0, i == 1, i == 2, i == 3,
                                               i == 4, i == 5, i == 6};
      TestBsdfReader test_reader(
          /*is_bsdf=*/true,
          /*uses_harmonic_extrapolation=*/false,
          /*num_elevational_samples=*/1,
          /*num_basis_functions=*/1,
          /*num_coefficients=*/1,
          /*num_color_channels=*/1,
          /*longest_series_length=*/1,
          /*num_parameters=*/1,
          /*num_parameter_values=*/1,
          /*metadata_size_bytes=*/4,
          /*index_of_refraction=*/1.0f,
          /*roughness_top=*/1.0f,
          /*roughness_bottom=*/1.0f, parsed_parameters);

      if (parsed_parameters[0]) {
        EXPECT_CALL(test_reader, HandleElevationalSample(_))
            .WillRepeatedly(Return(std::expected<void, std::string>()));
      }

      if (parsed_parameters[1]) {
        EXPECT_CALL(test_reader, HandleSampleCount(_))
            .WillRepeatedly(Return(std::expected<void, std::string>()));
      }

      if (parsed_parameters[2]) {
        EXPECT_CALL(test_reader, HandleSamplePosition(_))
            .WillRepeatedly(Return(std::expected<void, std::string>()));
      }

      if (parsed_parameters[3]) {
        EXPECT_CALL(test_reader, HandleCdf(_))
            .WillRepeatedly(Return(std::expected<void, std::string>()));
      }

      if (parsed_parameters[4]) {
        EXPECT_CALL(test_reader, HandleSeries(_, _))
            .WillRepeatedly(Return(std::expected<void, std::string>()));
      }

      if (parsed_parameters[5]) {
        EXPECT_CALL(test_reader, HandleCoefficient(_))
            .WillRepeatedly(Return(std::expected<void, std::string>()));
      }

      if (parsed_parameters[6]) {
        EXPECT_CALL(test_reader, HandleMetadata(_))
            .WillRepeatedly(Return(std::expected<void, std::string>()));
      }

      std::stringstream stream(minimal_bsdf.substr(0, i));
      auto result = test_reader.ReadFrom(stream);
      ASSERT_FALSE(result);
      EXPECT_EQ("Unexpected EOF", result.error());
    }
  }
}

TEST(BsdfReader, NonFiniteBsdfFails) {
  for (size_t i = 0; i < 7; i++) {
    std::array<bool, 7> parsed_parameters = {i == 0, i == 1, i == 2, i == 3,
                                             i == 4, i == 5, i == 6};
    TestBsdfReader test_reader(
        /*is_bsdf=*/true,
        /*uses_harmonic_extrapolation=*/false,
        /*num_elevational_samples=*/1,
        /*num_basis_functions=*/1,
        /*num_coefficients=*/1,
        /*num_color_channels=*/1,
        /*longest_series_length=*/1,
        /*num_parameters=*/1,
        /*num_parameter_values=*/1,
        /*metadata_size_bytes=*/4,
        /*index_of_refraction=*/1.0f,
        /*roughness_top=*/1.0f,
        /*roughness_bottom=*/1.0f, parsed_parameters);

    EXPECT_CALL(test_reader, HandleElevationalSample(_))
        .WillRepeatedly(Return(std::expected<void, std::string>()));
    EXPECT_CALL(test_reader, HandleSampleCount(_))
        .WillRepeatedly(Return(std::expected<void, std::string>()));
    EXPECT_CALL(test_reader, HandleSamplePosition(_))
        .WillRepeatedly(Return(std::expected<void, std::string>()));
    EXPECT_CALL(test_reader, HandleCdf(_))
        .WillRepeatedly(Return(std::expected<void, std::string>()));
    EXPECT_CALL(test_reader, HandleSeries(_, _))
        .WillRepeatedly(Return(std::expected<void, std::string>()));
    EXPECT_CALL(test_reader, HandleCoefficient(_))
        .WillRepeatedly(Return(std::expected<void, std::string>()));
    EXPECT_CALL(test_reader, HandleMetadata(_))
        .WillRepeatedly(Return(std::expected<void, std::string>()));

    bool should_fail = false;
    if (parsed_parameters[0]) {
      should_fail = true;
    }

    if (parsed_parameters[2]) {
      should_fail = true;
    }

    if (parsed_parameters[3]) {
      should_fail = true;
    }

    if (parsed_parameters[5]) {
      should_fail = true;
    }

    std::stringstream stream(MakeNonFiniteBsdfFile(1.0f, 1.0f, 1.0f));
    auto result = test_reader.ReadFrom(stream);

    if (should_fail) {
      ASSERT_FALSE(result);
      EXPECT_EQ("Input contained a non-finite floating point value",
                result.error());
    } else {
      EXPECT_TRUE(result);
    }
  }
}

}  // namespace
}  // namespace libfbsdf
