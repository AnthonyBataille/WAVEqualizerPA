#include "filter.hpp"
#include <cmath>

Filter::Filter() = default;
Filter::~Filter() = default;
void Filter::compute_coefficients() {};

float Filter::operator() (const float in_sample) {
	float out_sample = 0.0f;

	auto b_it = _coef_b.crbegin();
	for (size_t i = 0; i < _last_samples_input.size() - 1; ++i, ++b_it) {
		_last_samples_input[i] = _last_samples_input[i + 1];
		out_sample += _last_samples_input[i] * (*b_it);
	}
	_last_samples_input[_last_samples_input.size() - 1] = in_sample;
	out_sample += (*b_it) * in_sample;
	auto a_it = _coef_a.crbegin();
	for (size_t i = 0; i < _last_samples_output.size() - 1; ++i, ++a_it) {
		out_sample += _last_samples_output[i] * (*a_it);
		_last_samples_output[i] = _last_samples_output[i + 1];
	}
	out_sample += _last_samples_output[_last_samples_output.size() - 1] * (*a_it);
	_last_samples_output[_last_samples_output.size() - 1] = out_sample;
	return out_sample;
}

void PNFilter::compute_coefficients() {
	// The current implementation is a second order IIR filter.
	const float sqrt_G = std::sqrt(_G);
	const float cos_omega_c = std::cos(_omega_c);
	const float tan_B_2 = std::tan(_BW / 2.0f);

	_coef_a[0] = (2.0f * sqrt_G * cos_omega_c) / (sqrt_G + tan_B_2);
	_coef_a[1] = -(sqrt_G - tan_B_2) / (sqrt_G + tan_B_2);

	_coef_b[0] = (sqrt_G + _G * tan_B_2) / (sqrt_G + tan_B_2);
	_coef_b[1] = -_coef_a[0];
	_coef_b[2] = (sqrt_G - _G * tan_B_2) / (sqrt_G + tan_B_2);
}

void PNFilter::updateGain(const float gain) {
	_G = gain;
	compute_coefficients();
}

PNFilter::PNFilter() : _BW(0.0F), _G(0.0F), _omega_c(0.0F) {}

PNFilter::PNFilter(const float Bandwidth, const float gain, const float center_frenquency, const float max_frequency) {
	_BW = Bandwidth / max_frequency * PI;
	_G = gain;
	_omega_c = center_frenquency / max_frequency * PI;
	_coef_a.resize(2);
	_coef_b.resize(3);

	compute_coefficients();

	_last_samples_input = std::vector<float>(_coef_b.size());
	_last_samples_output = std::vector<float>(_coef_a.size());
}

PNFilter::~PNFilter() = default;
