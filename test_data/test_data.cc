#include "test_data/test_data.h"

#include <exception>
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
  std::string path = "__main__/" + file_params.path;
  return std::ifstream(runfiles->Rlocation(path),
                       std::ios::in | std::ios::binary);
}

std::pair<std::string, FileParams> MakeTestDataFile(
    const std::string& file_name) {
  FileParams file_params{.path = "test_data/" + file_name + ".bsdf.gz"};
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
  for (status = inflate(&stream, Z_SYNC_FLUSH); status == Z_OK;
       status = inflate(&stream, Z_SYNC_FLUSH)) {
    for (char* c = decompressed.get();
         c != reinterpret_cast<char*>(stream.next_out); c++) {
      output += *c;
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
    MakeTestDataFile("ceramic"),
    MakeTestDataFile("coated_copper"),
    MakeTestDataFile("leather"),
    MakeTestDataFile("paint"),
    MakeTestDataFile("roughglass_alpha_0.2"),
    MakeTestDataFile("roughgold_alpha_0.2"),
};

std::unique_ptr<std::istream> OpenTestData(const std::string& filename) {
  std::ifstream infile = OpenRunfile(kTestDataFiles.at(filename));
  std::vector<char> buffer((std::istreambuf_iterator<char>(infile)),
                           std::istreambuf_iterator<char>());
  return std::make_unique<std::stringstream>(DecompressBytes(buffer));
}

}  // namespace testing
}  // namespace libfbsdf