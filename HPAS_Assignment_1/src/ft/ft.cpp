#include "ft.h"


/**
 * Performs the 1D DFT in quadratic complexity
 * @param in input data
 * @return the input data in Fourier space
 */
NDArray<std::complex<double>> dft_1d(NDArray<double> in) {
    NDArray<std::complex<double>> out = NDArray<std::complex<double>>::empty({in.shape[0]});

    const auto N = in.shape[0];

    for (int k = 0; k < out.shape[0]; k++) {
        for (int n = 0; n < N; n++) {
            out(k) += in(n) * exp(- I * (2 * M_PI * k / N * n));
        }
    }
    return out;
}


/**
 * Performs the inverse 1D DFT in quadratic complexity.
 * Assumes prior input comprised real numbers, like in dft_1d(NDArray<double>)
 * @param in input data in Fourier space
 * @return the inverse Fourier transform of the input data
 */
NDArray<double> idft_1d(NDArray<std::complex<double>> in) {
    NDArray<double> out = NDArray<double>::empty({in.shape[0]});

    const auto N = out.shape[0];

    for (int n = 0; n < N; n++) {
        std::complex<double> tmpOut {0, 0};
        for (int k = 0; k < out.shape[0]; k++) {
            tmpOut += in(k) * exp(I * (2 * M_PI * k / N * n));
        }
        out(n) = tmpOut.real() / N;
    }
    return out;
}

/**
 ====== TODO: IMPLEMENT ======
 * Compute the Fast Fourier Transform (FFT) of a 1D array using the recursive method.
 * @param in: the input array to perform the FFT on.
 * @return the input array in Fourier space
**/
NDArray<std::complex<double>> cooley_tukey_fft_1d(NDArray<double> in) {
    NDArray<std::complex<double>> out = NDArray<std::complex<double>>::empty({in.shape[0]});

    const auto N = out.shape[0];

    if (N == 1) { // FFT of a single point is itself
        out(0) = in(0);
        return out;
    } 
    NDArray<double> even = NDArray<double>::empty({N/2});
    NDArray<double> odd = NDArray<double>::empty({N/2});

    // split into odd and even    
    for (int i = 0; i < N; i++) {
        if (i % 2 == 0) { 
            even(i/2) = in(i);
        } else {
            odd(i/2)= in(i);
        }
    }

    // recursive calls
    auto E = cooley_tukey_fft_1d(even);
    auto O = cooley_tukey_fft_1d(odd);


    for (int k = 0; k < N/2; k++) { //combination loop
        std::complex<double> twiddle = exp(-I * (2.0 * M_PI * k / N));
        std::complex<double> t = twiddle * O(k);
        out(k) = E(k) + t;
        out(k + N/2) = E(k) - t;

    }


    return out;
}


/**
 ====== TODO: IMPLEMENT ======
 * Compute the Fast Fourier Transform (FFT) of a 1D array using the recursive method.
 * Note that this function can be copied from the one above.
 * There need to be two separate ones, one for complex input and one for regular input, but they are exactly the same.
 * @param in: the input array to perform the FFT on.
 * @return the input array in Fourier space
**/
NDArray<std::complex<double>> cooley_tukey_fft_1d(NDArray<std::complex<double>> in)
{
    NDArray<std::complex<double>> out = NDArray<std::complex<double>>::empty({in.shape[0]});

    const auto N = out.shape[0];

    if (N == 1) { // FFT of a single point is itself
        out(0) = in(0);
        return out;
    } 
    NDArray<std::complex<double>> even = NDArray<std::complex<double>>::empty({N/2});
    NDArray<std::complex<double>> odd = NDArray<std::complex<double>>::empty({N/2});

    // split into odd and even    
    for (int i = 0; i < N; i++) {
        if (i % 2 == 0) { 
            even(i/2) = in(i);
        } else {
            odd(i/2)= in(i);
        }
    }

    // recursive calls
    auto E = cooley_tukey_fft_1d(even);
    auto O = cooley_tukey_fft_1d(odd);


    for (int k = 0; k < N/2; k++) { //combination loop
        std::complex<double> twiddle = exp(-I * (2.0 * M_PI * k / N));
        std::complex<double> t = twiddle * O(k);
        out(k) = E(k) + t;
        out(k + N/2) = E(k) - t;

    }


    return out;
}


/**
 ====== TODO: IMPLEMENT ======
 * Compute the Inverse Fast Fourier Transform (IFFT) of a 1D array using the recursive method.
 * @param in: the input array to perform the IFFT on.
 * @return the inverse transformed array without normalization
**/
NDArray<std::complex<double>> cooley_tukey_ifft_1d(NDArray<std::complex<double>> in) {
    NDArray<std::complex<double>> out = NDArray<std::complex<double>>::empty({in.shape[0]});

    const auto N = out.shape[0];

    if (N == 1) { // FFT of a single point is itself
        out(0) = in(0);
        return out;
    } 
    NDArray<std::complex<double>> even = NDArray<std::complex<double>>::empty({N/2});
    NDArray<std::complex<double>> odd = NDArray<std::complex<double>>::empty({N/2});

    // split into odd and even    
    for (int i = 0; i < N; i++) {
        if (i % 2 == 0) { 
            even(i/2) = in(i);
        } else {
            odd(i/2)= in(i);
        }
    }

    // recursive calls
    auto E = cooley_tukey_ifft_1d(even);
    auto O = cooley_tukey_ifft_1d(odd);


    for (int k = 0; k < N/2; k++) { //combination loop
        std::complex<double> twiddle = exp(I * (2.0 * M_PI * k / N));
        std::complex<double> t = twiddle * O(k);
        out(k) = E(k) + t;
        out(k + N/2) = E(k) - t;

    }

    return out;
}

unsigned int bitReverse(unsigned int x, int log2n)
{
    int n = 0;
    for (int i = 0; i < log2n; i++)
    {
        n <<= 1;
        n |= (x & 1);
        x >>= 1;
    }
    return n;
}


/**
 ====== TODO: IMPLEMENT ======
 * Compute the Fast Fourier Transform (FFT) of a 1D array using the iterative method.
 * @param in: the input array to perform the FFT on.
 * @return the input array in Fourier space
**/
NDArray<std::complex<double>> iterative_fft_1d(NDArray<double> in)
{
    if (in.shape[0] == 1)
        return { std::complex<double>{ in(0), 0 } }; // casts the real value to complex

    int n = in.shape[0];
	int log2n = log2(n);

    NDArray<std::complex<double>> out = NDArray<std::complex<double>>::empty({ in.shape[0] });

    for (int i = 0; i < n; i++) {
        out(bitReverse(i, log2n))= in(i);
    }

    for (int size = 2; size <= n; size *=2) {
        int half_size = size / 2;
        
        for (int chunkstart = 0; chunkstart < n; chunkstart += size) {
            for (int k = 0; k < half_size; k++) {
                std::complex<double> twiddle = exp(-I * (2.0 * M_PI * k / size));
                std::complex<double> t = twiddle * out(chunkstart + k + half_size);
                std::complex<double> u = out(chunkstart + k); 
                out(chunkstart + k ) = u + t; 
                out(chunkstart + k + half_size) = u - t;
            }
        }
    }

    return out;
}

/**
 ====== TODO: IMPLEMENT ======
 * Compute the Fast Fourier Transform (FFT) of a 1D array using the iterative method.
 * Note that this function can be copied from the one above.
 * There need to be two separate ones, one for complex input and one for regular input, but they are exactly the same.
 * @param in: the input array to perform the FFT on.
 * @return the input array in Fourier space
**/
NDArray<std::complex<double>> iterative_fft_1d(NDArray<std::complex<double>> in)
{
    if (in.shape[0] == 1)
        return { in(0) };

    int n = in.shape[0];
    int log2n = log2(n);

    NDArray<std::complex<double>> out = NDArray<std::complex<double>>::empty({ in.shape[0] });

    for (int i = 0; i < n; i++) {
        out(bitReverse(i, log2n))= in(i);
    }

    for (int size = 2; size <= n; size *=2) {
        int half_size = size / 2;
        
        for (int chunkstart = 0; chunkstart < n; chunkstart += size) {
            for (int k = 0; k < half_size; k++) {
                std::complex<double> twiddle = exp(-I * (2.0 * M_PI * k / size));
                std::complex<double> t = twiddle * out(chunkstart + k + half_size);
                std::complex<double> u = out(chunkstart + k); 
                out(chunkstart + k ) = u + t; 
                out(chunkstart + k + half_size) = u - t;
            }
        }
    }

    return out;
}


/**
 ====== TODO: IMPLEMENT ======
 * Compute the Inverse Fast Fourier Transform (IFFT) of a 1D array using the iterative method.
 * @param in: the input array to perform the IFFT on.
 * @return the inverse transformed array without normalization
**/
NDArray<std::complex<double>> iterative_ifft_1d(NDArray<std::complex<double>> in)
{
    if (in.shape[0] == 1)
        return { in(0) }; // casts the real value to complex

    int n = in.shape[0];
    int log2n = log2(n);

    NDArray<std::complex<double>> out = NDArray<std::complex<double>>::empty({ in.shape[0] });

    for (int i = 0; i < n; i++) {
        out(bitReverse(i, log2n))= in(i);
    }

    for (int size = 2; size <= n; size *=2) {
        int half_size = size / 2;
        
        for (int chunkstart = 0; chunkstart < n; chunkstart += size) {
            for (int k = 0; k < half_size; k++) {
                std::complex<double> twiddle = exp(I * (2.0 * M_PI * k / size));
                std::complex<double> t = twiddle * out(chunkstart + k + half_size);
                std::complex<double> u = out(chunkstart + k); 
                out(chunkstart + k ) = u + t; 
                out(chunkstart + k + half_size) = u - t;
            }
        }
    }

    return out;
}

/**
 * Implements the 2D FFT using the 1D FFT functions previously implemented
 * @param in input data
 * @param useIterative (true by default) specify whether to use the iterative or recursive FFT
 * @return the input data in Fourier space
 */
NDArray<std::complex<double>> fft_2d(NDArray<double> in, bool useIterative) {

    NDArray<std::complex<double>> out = NDArray<std::complex<double>>::empty({ in.shape[0], in.shape[1] });
    NDArray<std::complex<double>> temp = NDArray<std::complex<double>>::empty({ in.shape[0], in.shape[1] });
    
    // FT each row
#pragma omp parallel for
    for (int k = 0; k < out.shape[0]; k++) {
        // copy for the current thread
        NDArray<double> tmp = NDArray<double>::empty({out.shape[1]});
        for (int i = 0; i < out.shape[1]; i++) {
            tmp(i) = in(k, i);
        }

        NDArray<std::complex<double>> tmpFt;
        if (useIterative) tmpFt = iterative_fft_1d(tmp);
		else tmpFt = cooley_tukey_fft_1d(tmp);

        // copy into the output array
        for (int i = 0; i < out.shape[1]; i++) {
            temp(k, i) = tmpFt(i);
        }
    }

    // FT each column
#pragma omp parallel for
    for (int i = 0; i < out.shape[1]; i++) {
        // copy for the current thread
        NDArray<std::complex<double>> tmp = NDArray<std::complex<double>>::empty({out.shape[0]});
        for (int k = 0; k < out.shape[0]; k++) {
            tmp(k) = temp(k, i);
        }


        NDArray<std::complex<double>> tmpFt;
        if (useIterative) tmpFt = iterative_fft_1d(tmp);
        else tmpFt = cooley_tukey_fft_1d(tmp);

        // copy into the output array
        for (int k = 0; k < out.shape[0]; k++) {
            out(k, i) = tmpFt(k);
        }
    }

    return out;
}

/**
 * Implements the inverse 2D FFT using the 1D FFT functions previously implemented
 * @param in input data
 * @param useIterative (true by default) specify whether to use the iterative or recursive FFT
 * @return the input data in Fourier space
 */
NDArray<double> ifft_2d(NDArray<std::complex<double>> in, bool useIterative) {

    NDArray<std::complex<double>> outCplx = NDArray<std::complex<double>>::empty({in.shape[0], in.shape[1]});
    const auto N = in.shape[0] * in.shape[1];

    // FT each row
#pragma omp parallel for
    for (int y = 0; y < outCplx.shape[0]; y++) {
        // copy for the current thread
        NDArray<std::complex<double>> tmp = NDArray<std::complex<double>>::empty({outCplx.shape[1]});
        for (int x = 0; x < outCplx.shape[1]; x++) {
            tmp(x) = in(y, x);
        }

        NDArray<std::complex<double>> tmpFt;
        if (useIterative) tmpFt = iterative_ifft_1d(tmp);
        else tmpFt = cooley_tukey_ifft_1d(tmp);

        // copy into the output array
        for (int x = 0; x < outCplx.shape[1]; x++) {
            outCplx(y, x) = tmpFt(x);
        }
    }

    NDArray<double> out = NDArray<double>::empty({in.shape[0], in.shape[1]});
    // FT each column
#pragma omp parallel for
    for (int x = 0; x < outCplx.shape[1]; x++) {
        // copy for the current thread
        NDArray<std::complex<double>> tmp = NDArray<std::complex<double>>::empty({outCplx.shape[0]});
        for (int y = 0; y < outCplx.shape[0]; y++) {
            tmp(y) = outCplx(y, x);
        }

        NDArray<std::complex<double>> tmpFt;
        if (useIterative) tmpFt = iterative_ifft_1d(tmp);
        else tmpFt = cooley_tukey_ifft_1d(tmp);

        // copy into the output array
        for (int y = 0; y < outCplx.shape[0]; y++) {
            out(y, x) = tmpFt(y).real() / N;
        }
    }

    return out;
}

/**
 * Performs the 2D DFT in quadratic complexity
 * @param in input data
 * @return the input data in Fourier space
 */
NDArray<std::complex<double>> dft_2d(NDArray<double> in)
{
	NDArray<std::complex<double>> out = NDArray<std::complex<double>>::empty({ in.shape[0], in.shape[1] });

	for (int k = 0; k < out.shape[0]; k++) {
		for (int l = 0; l < out.shape[1]; l++) {
			for (int m = 0; m < in.shape[0]; m++) {
				for (int n = 0; n < in.shape[1]; n++) {
					out(k, l) += in(m, n) * exp(-I * (2 * M_PI * (k * m / (double)in.shape[0] + l * n / (double)in.shape[1])));
				}
			}
		}
	}

	return out;
}

/**
 * Performs the inverse 2D DFT in quadratic complexity.
 * Assumes prior input comprised real numbers, like in dft_2d(NDArray<double>)
 * @param in input data in Fourier space
 * @return the inverse Fourier transform of the input data
 */
NDArray<double> idft_2d(NDArray<std::complex<double>> in)
{
	NDArray<double> out = NDArray<double>::empty({ in.shape[0], in.shape[1] });
    const auto N = (in.shape[0] * in.shape[1]);

	for (int m = 0; m < out.shape[0]; m++) {
		for (int n = 0; n < out.shape[1]; n++) {
			std::complex<double> tmpOut {0, 0};
			for (int k = 0; k < in.shape[0]; k++) {
				for (int l = 0; l < in.shape[1]; l++) {
					tmpOut += in(k, l) * exp(I * (2 * M_PI * (k * m / (double)in.shape[0] + l * n / (double)in.shape[1])));
				}
			}
			out(m, n) = tmpOut.real() / N;
		}
	}

	return out;
}
