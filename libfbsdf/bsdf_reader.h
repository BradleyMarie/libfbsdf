#ifndef _LIBFBSDF_BSDF_READER_
#define _LIBFBSDF_BSDF_READER_

#include <cstddef>
#include <cstdint>
#include <expected>
#include <istream>
#include <string>

namespace libfbsdf {

// The base class for reading Fourier BSDF fomatted inputs. This class does very
// little validation of the inputs other than insuring that all of the data
// described in the header is present in the input and that any floating point
// values contained in the input are finite values.
class BsdfReader {
 public:
  // NOTE: Behavior is undefined if input is not a binary stream
  std::expected<void, std::string> ReadFrom(std::istream& input);

 protected:
  // Flags from the header of the input.
  struct Flags {
    bool is_bsdf;
    bool uses_harmonic_extrapolation;
  };

  // Controls parts of the input that are read during parsing.  Any parts marked
  // as unparsed will be entirely skipped and will not have their corresponding
  // callbacks called.
  struct Options {
    bool parse_elevational_samples = true;
    bool parse_parameter_sample_counts = true;
    bool parse_parameter_values = true;
    bool parse_cdf_mu = true;
    bool parse_series = true;
    bool parse_coefficients = true;
    bool parse_metadata = true;
  };

  // Called at the start of parsing an input and passes information parsed from
  // the header of the input. Returns the parts of the input that should
  // be parsed or an error if the input cannot be read by the reader.
  virtual std::expected<Options, std::string> Start(
      const Flags& flags, size_t num_elevational_samples,
      size_t num_basis_functions, size_t num_coefficients,
      size_t num_color_channels, size_t longest_series_length,
      size_t num_parameters, size_t num_parameter_values,
      size_t metadata_size_bytes, float index_of_refraction,
      float roughness_top, float roughness_bottom) = 0;

  // Provides the list of the the elevational samples in the order that they
  // appear in the input. Returns an error if the input cannot be read by the
  // reader.
  //
  // Will be called once per value in the input.
  virtual std::expected<void, std::string> HandleElevationalSample(
      float value) {
    return std::expected<void, std::string>();
  }

  // TODO: Document what this provides
  // Returns an error if the input cannot be read by the reader.
  //
  // Will be called once per value in the input.
  virtual std::expected<void, std::string> HandleSampleCount(uint32_t value) {
    return std::expected<void, std::string>();
  }

  // TODO: Document what this provides
  // Returns an error if the input cannot be read by the reader.
  //
  // Will be called once per value in the input.
  virtual std::expected<void, std::string> HandleSamplePosition(float value) {
    return std::expected<void, std::string>();
  }

  // Provides the list of the the CDF values in the order that they appear in
  // the input. Returns an error if the input cannot be read by the reader.
  //
  // Will be called once per value in the input.
  virtual std::expected<void, std::string> HandleCdf(float value) {
    return std::expected<void, std::string>();
  }

  // Provides the list of the Fourier series present in the input in the order
  // in which they are specified in the input without any bounds or sanity
  // checking. Returns an error if the input cannot be read by the reader.
  //
  // Will be called once per Fourier series in the input.
  virtual std::expected<void, std::string> HandleSeries(uint32_t offset,
                                                        uint32_t length) {
    return std::expected<void, std::string>();
  }

  // Provides the list of the the Fourier coefficients in the order that they
  // appear in the input. Returns an error if the input cannot be read by the
  // reader.
  //
  // Will be called once per value in the input.
  virtual std::expected<void, std::string> HandleCoefficient(float value) {
    return std::expected<void, std::string>();
  }

  // Provides the metadata in the input as a string. Returns an error if the
  // input cannot be read by the reader.
  //
  // Will be called once per input, if present.
  virtual std::expected<void, std::string> HandleMetadata(std::string data) {
    return std::expected<void, std::string>();
  }

  // Called at the end of parsing after all values have been handled. Returns an
  // error if the input cannot be read by the reader.
  virtual std::expected<void, std::string> Finish() {
    return std::expected<void, std::string>();
  }
};

}  // namespace libfbsdf

#endif  // _LIBFBSDF_BSDF_READER_
