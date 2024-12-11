#ifndef _TEST_DATA_TEST_DATA_
#define _TEST_DATA_TEST_DATA_

#include <cstdint>
#include <filesystem>
#include <istream>
#include <map>
#include <memory>
#include <string>

#include "libfbsdf/bsdf_reader.h"

namespace libfbsdf {
namespace testing {

struct FileParams {
  std::filesystem::path path;
  bool is_bsdf;
  bool uses_harmonic_extrapolation;
  uint32_t num_elevational_samples;
  uint32_t num_basis_functions;
  uint32_t num_coefficients;
  uint32_t num_color_channels;
  uint32_t longest_series_length;
  uint32_t num_parameters;
  uint32_t num_parameter_values;
  uint32_t metadata_size_bytes;
  float index_of_refraction;
  float roughness_top;
  float roughness_bottom;
};

// A map from the name of a test data file to its parameters
extern const std::map<std::string, FileParams> kTestDataFiles;

// Open a test data file by name into an istream
std::unique_ptr<std::istream> OpenTestData(const std::string& filename);

}  // namespace testing
}  // namespace libfbsdf

#endif  // _TEST_DATA_TEST_DATA_