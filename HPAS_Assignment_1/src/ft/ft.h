#pragma once
#include <functional>
#include <complex>
#include <cassert>

#define _USE_MATH_DEFINES
#include <math.h>

#include <framework/NDArray.h>

/**
 * Definition of the imaginary unit.
 */
const std::complex<double> I(0.0, 1.0);

NDArray<std::complex<double>> dft_1d(NDArray<double> in);

NDArray<double> idft_1d(NDArray<std::complex<double>> in);

NDArray<std::complex<double>> cooley_tukey_fft_1d(NDArray<double> in);

NDArray<std::complex<double>> cooley_tukey_ifft_1d(NDArray<std::complex<double>> in);

NDArray<std::complex<double>> iterative_fft_1d(NDArray<double> in);

NDArray<std::complex<double>> iterative_ifft_1d(NDArray<std::complex<double>> in);

NDArray<std::complex<double>> fft_2d(NDArray<double> in, bool useIterative = true);

NDArray<double> ifft_2d(NDArray<std::complex<double>> in, bool useIterative = true);

NDArray<std::complex<double>> dft_2d(NDArray<double> in);

NDArray<double> idft_2d(NDArray<std::complex<double>> in);

NDArray<std::complex<double>> cooley_tukey_fft_1d(NDArray<std::complex<double>> in);

NDArray<std::complex<double>> iterative_fft_1d(NDArray<std::complex<double>> in);
