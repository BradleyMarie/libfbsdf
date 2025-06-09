#include "test_data/test_data.h"

#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <istream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "third_party/zlib/zlib.h"
#include "tools/cpp/runfiles/runfiles.h"

namespace libfbsdf {
namespace testing {
namespace {

using ::bazel::tools::cpp::runfiles::Runfiles;

std::ifstream OpenRunfile(const FileParams& file_params) {
  std::unique_ptr<Runfiles> runfiles(Runfiles::CreateForTest());
  return std::ifstream(runfiles->Rlocation(file_params.path.string()),
                       std::ios::in | std::ios::binary);
}

std::pair<std::string, FileParams> MakeTestDataFile(
    const std::string& file_name, bool is_bsdf,
    bool uses_harmonic_extrapolation, uint32_t num_elevational_samples,
    uint32_t num_basis_functions, uint32_t num_coefficients,
    uint32_t num_color_channels, uint32_t longest_series_length,
    uint32_t num_parameters, uint32_t num_parameter_values,
    uint32_t metadata_size_bytes, float index_of_refraction,
    float roughness_top, float roughness_bottom) {
  FileParams file_params{
      .path = std::filesystem::canonical("test_data/" + file_name + ".bsdf.gz"),
      .is_bsdf = is_bsdf,
      .uses_harmonic_extrapolation = uses_harmonic_extrapolation,
      .num_elevational_samples = num_elevational_samples,
      .num_basis_functions = num_basis_functions,
      .num_coefficients = num_coefficients,
      .num_color_channels = num_color_channels,
      .longest_series_length = longest_series_length,
      .num_parameters = num_parameters,
      .num_parameter_values = num_parameter_values,
      .metadata_size_bytes = metadata_size_bytes,
      .index_of_refraction = index_of_refraction,
      .roughness_top = roughness_top,
      .roughness_bottom = roughness_bottom,
  };
  return std::make_pair(file_name, std::move(file_params));
}

std::string DecompressBytes(const std::vector<char>& compressed_bytes) {
  static constexpr size_t kChunkSize = 524288;  // 512KB
  auto decompressed = std::make_unique<char[]>(kChunkSize);

  z_stream stream;
  stream.zalloc = Z_NULL;
  stream.zfree = Z_NULL;
  stream.opaque = Z_NULL;
  stream.avail_in = compressed_bytes.size();
  stream.next_in = const_cast<Bytef*>(
      reinterpret_cast<const Bytef*>(compressed_bytes.data()));
  stream.avail_out = kChunkSize;
  stream.next_out = reinterpret_cast<Bytef*>(decompressed.get());

  int status = inflateInit2(&stream, 16 + MAX_WBITS);
  if (status != Z_OK) {
    throw std::runtime_error("inflateInit failed");
  }

  std::string output;
  for (;;) {
    status = inflate(&stream, Z_SYNC_FLUSH);

    for (char* c = decompressed.get();
         c != reinterpret_cast<char*>(stream.next_out); c++) {
      output += *c;
    }

    if (status != Z_OK) {
      break;
    }

    stream.avail_out = kChunkSize;
    stream.next_out = reinterpret_cast<Bytef*>(decompressed.get());
  }

  inflateEnd(&stream);

  if (status != Z_STREAM_END) {
    throw std::runtime_error("inflate failed");
  }

  return output;
}

};  // namespace

const std::map<std::string, FileParams> kTestDataFiles = {
    MakeTestDataFile(/*file_name=*/"ceramic", /*is_bsdf=*/1,
                     /*uses_harmonic_extrapolation=*/0,
                     /*num_elevational_samples=*/852,
                     /*num_basis_functions=*/1,
                     /*num_coefficients=*/24360150,
                     /*num_color_channels=*/3,
                     /*longest_series_length=*/1599,
                     /*num_parameters=*/0,
                     /*num_parameter_values=*/0,
                     /*metadata_size_bytes=*/0,
                     /*index_of_refraction=*/1,
                     /*roughness_top=*/0,
                     /*roughness_bottom=*/0),
    MakeTestDataFile(/*file_name=*/"coated_copper", /*is_bsdf=*/1,
                     /*uses_harmonic_extrapolation=*/0,
                     /*num_elevational_samples=*/328,
                     /*num_basis_functions=*/1,
                     /*num_coefficients=*/2331240,
                     /*num_color_channels=*/3,
                     /*longest_series_length=*/530,
                     /*num_parameters=*/0,
                     /*num_parameter_values=*/0,
                     /*metadata_size_bytes=*/953,
                     /*index_of_refraction=*/1,
                     /*roughness_top=*/0,
                     /*roughness_bottom=*/0),
    MakeTestDataFile(/*file_name=*/"leather", /*is_bsdf=*/1,
                     /*uses_harmonic_extrapolation=*/0,
                     /*num_elevational_samples=*/94,
                     /*num_basis_functions=*/1,
                     /*num_coefficients=*/70950,
                     /*num_color_channels=*/3,
                     /*longest_series_length=*/61,
                     /*num_parameters=*/0,
                     /*num_parameter_values=*/0,
                     /*metadata_size_bytes=*/0,
                     /*index_of_refraction=*/1,
                     /*roughness_top=*/0,
                     /*roughness_bottom=*/0),
    MakeTestDataFile(/*file_name=*/"paint", /*is_bsdf=*/1,
                     /*uses_harmonic_extrapolation=*/0,
                     /*num_elevational_samples=*/102,
                     /*num_basis_functions=*/1,
                     /*num_coefficients=*/95991,
                     /*num_color_channels=*/3,
                     /*longest_series_length=*/74,
                     /*num_parameters=*/0,
                     /*num_parameter_values=*/0,
                     /*metadata_size_bytes=*/0,
                     /*index_of_refraction=*/1,
                     /*roughness_top=*/0,
                     /*roughness_bottom=*/0),
    MakeTestDataFile(/*file_name=*/"roughglass_alpha_0.2", /*is_bsdf=*/1,
                     /*uses_harmonic_extrapolation=*/0,
                     /*num_elevational_samples=*/114,
                     /*num_basis_functions=*/1,
                     /*num_coefficients=*/190440,
                     /*num_color_channels=*/1,
                     /*longest_series_length=*/92,
                     /*num_parameters=*/0,
                     /*num_parameter_values=*/0,
                     /*metadata_size_bytes=*/309,
                     /*index_of_refraction=*/1.5046,
                     /*roughness_top=*/0,
                     /*roughness_bottom=*/0),
    MakeTestDataFile(/*file_name=*/"roughgold_alpha_0.2", /*is_bsdf=*/1,
                     /*uses_harmonic_extrapolation=*/0,
                     /*num_elevational_samples=*/58,
                     /*num_basis_functions=*/1,
                     /*num_coefficients=*/41502,
                     /*num_color_channels=*/3,
                     /*longest_series_length=*/172,
                     /*num_parameters=*/0,
                     /*num_parameter_values=*/0,
                     /*metadata_size_bytes=*/682,
                     /*index_of_refraction=*/1,
                     /*roughness_top=*/0,
                     /*roughness_bottom=*/0),
};

std::unique_ptr<std::istream> OpenTestData(const std::string& filename) {
  std::ifstream infile = OpenRunfile(kTestDataFiles.at(filename));
  std::vector<char> buffer((std::istreambuf_iterator<char>(infile)),
                           std::istreambuf_iterator<char>());
  return std::make_unique<std::stringstream>(DecompressBytes(buffer));
}

}  // namespace testing
}  // namespace libfbsdf
