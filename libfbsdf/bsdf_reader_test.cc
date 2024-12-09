#include "libfbsdf/bsdf_reader.h"

#include <array>
#include <istream>
#include <memory>
#include <string>

#include "googlemock/include/gmock/gmock.h"
#include "googletest/include/gtest/gtest.h"
#include "test_data/test_data.h"

namespace libfbsdf {
namespace {

using ::libfbsdf::testing::FileParams;
using ::libfbsdf::testing::kTestDataFiles;
using ::libfbsdf::testing::OpenTestData;
using ::testing::_;
using ::testing::Return;

class TestBsdfReader : public BsdfReader {
 public:
  TestBsdfReader(const FileParams& file_params,
                 const std::array<bool, 7>& parsed_parameters)
      : file_params_(file_params), parsed_parameters_(parsed_parameters) {}

  std::expected<Options, std::string> Start(
      const Flags& flags, size_t num_elevational_samples,
      size_t num_basis_functions, size_t num_coefficients,
      size_t num_color_channels, size_t longest_series_length,
      size_t num_parameters, size_t num_parameter_values,
      size_t metadata_size_bytes, float index_of_refraction,
      float roughness_top, float roughness_bottom) {
    EXPECT_EQ(file_params_.is_bsdf, flags.is_bsdf);
    EXPECT_EQ(file_params_.uses_harmonic_extrapolation,
              flags.uses_harmonic_extrapolation);
    EXPECT_EQ(file_params_.num_elevational_samples, num_elevational_samples);
    EXPECT_EQ(file_params_.num_basis_functions, num_basis_functions);
    EXPECT_EQ(file_params_.num_coefficients, num_coefficients);
    EXPECT_EQ(file_params_.num_color_channels, num_color_channels);
    EXPECT_EQ(file_params_.longest_series_length, longest_series_length);
    EXPECT_EQ(file_params_.num_parameters, num_parameters);
    EXPECT_EQ(file_params_.num_parameter_values, num_parameter_values);
    EXPECT_EQ(file_params_.metadata_size_bytes, metadata_size_bytes);
    EXPECT_EQ(file_params_.index_of_refraction, index_of_refraction);
    EXPECT_EQ(file_params_.roughness_top, roughness_top);
    EXPECT_EQ(file_params_.roughness_bottom, roughness_bottom);

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
  MOCK_METHOD((std::expected<void, std::string>), Finish, (), (override));

 private:
  FileParams file_params_;
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

      EXPECT_CALL(test_reader, Finish())
          .WillOnce(Return(std::expected<void, std::string>()));

      EXPECT_TRUE(test_reader.ReadFrom(*OpenTestData(file_name)));
    }
  }
}

}  // namespace
}  // namespace libfbsdf