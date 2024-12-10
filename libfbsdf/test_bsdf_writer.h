#ifndef _LIBFBSDF_TEST_BSDF_WRITER_
#define _LIBFBSDF_TEST_BSDF_WRITER_

#include <cstdint>
#include <string>
#include <vector>

namespace libfbsdf {
namespace testing {

struct Flags {
  bool is_bsdf;
  bool uses_harmonic_extrapolation;
};

class BsdfData {
 public:
  BsdfData(std::vector<float> elevational_samples, size_t num_basis_functions,
           size_t num_channels);

  const std::vector<float>& GetElevationalSamples() const {
    return elevational_samples_;
  }

  void AddCoefficient(size_t channel, size_t sample_x, size_t sample_y,
                      float value);

  struct Coefficients {
    std::vector<uint32_t> bounds;
    std::vector<float> coefficients;
    size_t max_order;
  };

  Coefficients SerializeCoefficients() const;

  void SetCdf(size_t basis_function, size_t sample_x, size_t sample_y,
              float value);

  const std::vector<float>& GetCdf() const { return cdf_; }

  size_t GetNumBasisFunctions() const { return num_basis_functions_; }

  size_t GetNumChannels() const { return num_channels_; }

 private:
  std::vector<float> elevational_samples_;
  std::vector<std::vector<float>> series_;
  std::vector<float> cdf_;
  size_t num_basis_functions_;
  size_t num_channels_;
};

std::string MakeBsdfFile(const Flags& flags, const BsdfData& bsdf_data,
                         std::vector<uint32_t> parameter_sample_counts,
                         std::vector<float> parameters, std::string metadata,
                         float index_of_refraction, float roughness_top,
                         float roughness_bottom);

std::string MakeEmptyBsdfFile(float index_of_refraction, float roughness_top,
                              float roughness_bottom);

std::string MakeMinimalBsdfFile(float index_of_refraction, float roughness_top,
                                float roughness_bottom);

std::string MakeNonFiniteBsdfFile(float index_of_refraction,
                                  float roughness_top, float roughness_bottom);

}  // namespace testing
}  // namespace libfbsdf

#endif  // _LIBFBSDF_TEST_BSDF_WRITER_