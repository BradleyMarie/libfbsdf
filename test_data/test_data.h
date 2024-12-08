#ifndef _TEST_DATA_TEST_DATA_
#define _TEST_DATA_TEST_DATA_

#include <istream>
#include <map>
#include <memory>
#include <string>

#include "libfbsdf/bsdf_reader.h"

namespace libfbsdf {
namespace testing {

struct FileParams {
  std::string path;
};

// A map from the name of a test data file to its parameters
extern const std::map<std::string, FileParams> kTestDataFiles;

// Open a test data file by name into an istream
std::unique_ptr<std::istream> OpenTestData(const std::string& filename);

}  // namespace testing
}  // namespace libfbsdf

#endif  // _TEST_DATA_TEST_DATA_