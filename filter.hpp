#pragma once

#include <vector>

constexpr float PI = 3.141592f;

/**
 * @brief Generic IIR filter that can specialize as a peak-notch or shelving
 * filter. It features a computation of output sample via overload of operator()
 * and automatic handling of last samples memory.
 */
class Filter {
protected:
  std::vector<float> _last_samples_input;
  std::vector<float> _last_samples_output;

  /**
   * @brief Feedback coefficients.
   */
  std::vector<float> _coef_a;
  /**
   * @brief Feedforward coefficients.
   */
  std::vector<float> _coef_b;

  void compute_coefficients();

public:
  float Filter::operator()(const float in_sample);
  Filter();
  ~Filter();
};

/**
 * @brief Second order peaking-notch IIR filter defined via the following
 parameters: Bandwidth in Hz Gain Center Frequency in Hz Maxuimum frequency
 (Nyquist fequency) in Hz.
 */
class PNFilter : public Filter {
protected:
  /**
   * @brief Bandwidth in normalized [0, 2*pi] range.
   */
  float _BW;
  /**
   * @brief Filter gain (at center frequency).
   */
  float _G;
  /**
   * @brief Center frequency in normalized [0, 2*pi] range.
   */
  float _omega_c;

  /**
   * @brief computes the feedback (a) and feedforward (b) filter coefficients
   * given the input parameters.
   */
  void compute_coefficients();

public:
  void updateGain(const float gain);
  PNFilter();
  PNFilter(const float Bandwidth, const float gain,
           const float center_frequency, const float max_frequency);
  ~PNFilter();
};

/**
 * @brief limiter function to protect against saturation.
 * @param in input sample.
 * @param threshold percentage as maximum level that is set as limit.
 */
float limiter(const float in, const float threshold);
