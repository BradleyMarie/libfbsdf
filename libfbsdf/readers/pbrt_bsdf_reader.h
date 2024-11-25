#ifndef _LIBFBSDF_READERS_PBRT_BSDF_READER_
#define _LIBFBSDF_READERS_PBRT_BSDF_READER_

#include <cstdint>
#include <expected>
#include <istream>
#include <memory>
#include <string>
#include <vector>

#include "libfbsdf/bsdf_reader.h"

namespace libfbsdf {

// A BSDF reader that only reads the components of Fourier BSDF file relevant
// for rendering in PBRT
class PbrtBsdfReader final : public BsdfReader {
 public:
  float IndexOfRefraction() const { return index_of_refraction_; }

  size_t MaxOrder() const { return mMax_; }

  size_t ColorChannels() const { return nChannels_; }

  std::shared_ptr<const std::vector<float>> Mu() const { return mu_; }

  std::shared_ptr<const std::vector<float>> Cdf() const { return cdf_; }

  std::shared_ptr<const std::vector<uint32_t>> M() const { return m_; }

  std::shared_ptr<const std::vector<uint32_t>> AOffset() const {
    return aOffset_;
  }

  std::shared_ptr<const std::vector<float>> A() const { return a_; }

  std::shared_ptr<const std::vector<float>> a0() const { return a0_; }

 protected:
  std::expected<Options, std::string> Start(
      const Flags& flags, size_t num_nodes, size_t num_basis_functions,
      size_t num_coefficients, size_t num_color_channels, size_t num_max_order,
      size_t num_parameters, size_t num_parameter_values,
      size_t metadata_size_bytes, float eta, float alpha_top,
      float alpha_bottom) override;

  std::expected<void, std::string> HandleNode(float value) override;

  std::expected<void, std::string> HandleCdfMu(float value) override;

  std::expected<void, std::string> HandleOffsetAndLength(
      uint32_t offset, uint32_t length) override;

  std::expected<void, std::string> HandleCoefficient(float value) override;

  std::expected<void, std::string> Finish() override;

 private:
  std::shared_ptr<std::vector<float>> mu_ =
      std::make_shared<std::vector<float>>();
  std::shared_ptr<std::vector<float>> cdf_ =
      std::make_shared<std::vector<float>>();
  std::shared_ptr<std::vector<uint32_t>> m_ =
      std::make_shared<std::vector<uint32_t>>();
  std::shared_ptr<std::vector<uint32_t>> aOffset_ =
      std::make_shared<std::vector<uint32_t>>();
  std::shared_ptr<std::vector<float>> a_ =
      std::make_shared<std::vector<float>>();
  std::shared_ptr<std::vector<float>> a0_ =
      std::make_shared<std::vector<float>>();

  float index_of_refraction_ = 1.0f;
  uint32_t mMax_ = 0u;
  uint32_t nChannels_ = 0u;
};

}  // namespace libfbsdf

#endif  // _LIBFBSDF_READERS_SIMPLE_BSDF_READER_