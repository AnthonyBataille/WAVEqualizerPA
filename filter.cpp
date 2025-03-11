#include "filter.hpp"
#include <cmath>

Filter::Filter() = default;
Filter::~Filter() = default;
void Filter::compute_coefficients() {};

float Filter::operator() (const float in_sample) {
	float out_sample = 0.0f;
	_last_samples_input.pop_front();
	_last_samples_input.push_back(in_sample);
	auto b_it = _coef_b.cbegin();
	for (auto input_it = _last_samples_input.crbegin(); input_it != _last_samples_input.crend(); ++input_it, ++b_it) {
		out_sample += (*input_it) * (*b_it);
	}
	auto a_it = _coef_a.cbegin();
	for (auto output_it = _last_samples_output.crbegin(); output_it != _last_samples_output.crend(); ++output_it, ++a_it) {
		out_sample += (*output_it) * (*a_it);
	}
	_last_samples_output.pop_front();
	_last_samples_output.push_back(out_sample);
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


PNFilter::PNFilter(const float Bandwidth, const float gain, const float center_frenquency, const float max_frequency) {
	_BW = Bandwidth / max_frequency * PI;
	_G = gain;
	_omega_c = center_frenquency / max_frequency * PI;
	_coef_a.resize(2);
	_coef_b.resize(3);

	compute_coefficients();

	_last_samples_input = std::list<float>(_coef_b.size());
	_last_samples_output = std::list<float>(_coef_a.size());
}

PNFilter::~PNFilter() = default;
