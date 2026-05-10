#include <iostream>
#include <catch2/catch_all.hpp>
#include <random>

#include <ft/ft.h>

//#define RUN_BENCHMARKS // Uncomment if you would like to run the example benchmark
//#define FFT_2D_TEST_OUT

std::vector<double> generate_random_vector(size_t size) {
    std::vector<double> data(size);

    // Random data generation based on https://stackoverflow.com/questions/21102105/random-double-c11
    const double lower_bound = 0;
    const double upper_bound = 1;
    std::uniform_real_distribution<double> unif(lower_bound, upper_bound);

    std::random_device rand_dev;              // Use random_device to get a random seed.

    std::mt19937 rand_engine(rand_dev()); // mt19937 is a good pseudo-random number generator

    for (int i = 0; i < size; i++) {
        data[i] = unif(rand_engine);
    }

    return data;
}


TEST_CASE("DFT Idempotence Large Random Data", "[dft-1d]") {
    auto size = GENERATE(1, 2, 4, 8, 255, 256, 500, 1024);

    auto data = generate_random_vector(size);

    NDArray<double> fromData(data);

    auto dftTestOut = dft_1d(fromData);
    auto idftTestOut = idft_1d(dftTestOut);

    REQUIRE(data.size() == dftTestOut.shape[0]);
    REQUIRE(dftTestOut.shape[0] == idftTestOut.shape[0]);

    REQUIRE(fromData.approxEquals(idftTestOut));

}

#ifdef RUN_BENCHMARKS
TEST_CASE("DFT/FFT Performance", "[dft-1d]") {
    size_t size = 1024;

    auto data = generate_random_vector(size);

    NDArray<double> fromData{ data };

    BENCHMARK("DFT of 1024 points") {
        return dft_1d(fromData);
    };
}
#endif

TEST_CASE("2D DFT Idempotence, square", "[dft-2d]") {
    size_t size = 64;

    auto data = generate_random_vector(size);

    NDArray<double> in(data);
    in.reshape(8, 8);

#ifdef FFT_2D_TEST_OUT
    std::cout << "Input data:\n";
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            std::cout << in(i, j) << " ";
        }
    }
#endif

    auto dftTestOut = dft_2d(in);

#ifdef FFT_2D_TEST_OUT
    std::cout << "Fourier domain of data:\n";
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            std::cout << dftTestOut(i, j) << " ";
        }
    }
    std::cout << std::endl;
#endif
    auto idftTestOut = idft_2d(dftTestOut);

#ifdef FFT_2D_TEST_OUT
    std::cout << "IFFT output:\n";
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            std::cout << idftTestOut(i, j) << " ";
        }
    }
    std::cout << std::endl;
#endif

    REQUIRE(idftTestOut.approxEquals(in));

}

TEST_CASE("Recursive FFT (real) matches DFT", "[fft-1d]") {
    auto size = GENERATE(1, 2, 4, 8, 256, 1024);

    auto data = generate_random_vector(size);

    NDArray<double> fromData(data);

    auto dft_out = dft_1d(fromData);
    auto fft_out = cooley_tukey_fft_1d(fromData);

    REQUIRE(dft_out.shape[0] == fft_out.shape[0]);
    REQUIRE(dft_out.approxEquals(fft_out));

}

TEST_CASE("Recursive FFT (complex) matches DFT", "[fft-1d]") {
    auto size = GENERATE(1, 2, 4, 8, 256, 1024);

    auto data = generate_random_vector(size);

    NDArray<double> fromData(data);

    NDArray<std::complex<double>> complex_in = NDArray<std::complex<double>>::empty({size});
    for (size_t i = 0; i < size; i++) {
        complex_in(i) = fromData(i);
    }

    auto dft_out = dft_1d(fromData);
    auto fft_out = cooley_tukey_fft_1d(complex_in);

    REQUIRE(dft_out.shape[0] == fft_out.shape[0]);
    REQUIRE(dft_out.approxEquals(fft_out));

}

// Verifies the recursive IFFT against the verified IDFT. The recursive IFFT
// returns an un-normalized result, so we divide by N before comparing.
TEST_CASE("Recursive IFFT matches IDFT", "[ifft-1d]") {
    auto size = GENERATE(1, 2, 4, 8, 256, 1024);

    auto data = generate_random_vector(size);
    NDArray<double> fromData(data);

    auto freq = dft_1d(fromData);
    auto idft_out = idft_1d(freq);
    auto ifft_out = cooley_tukey_ifft_1d(freq);

    REQUIRE(idft_out.shape[0] == ifft_out.shape[0]);

    NDArray<double> ifft_normalized = NDArray<double>::empty({size});
    for (int i = 0; i < size; i++) {
        ifft_normalized(i) = ifft_out(i).real() / size;
    }

    REQUIRE(idft_out.approxEquals(ifft_normalized));
}

// Verifies the iterative FFT on real input produces the same Fourier
// coefficients as the verified quadratic DFT.
TEST_CASE("Iterative FFT (real) matches DFT", "[fft-1d]") {
    auto size = GENERATE(1, 2, 4, 8, 256, 1024);

    auto data = generate_random_vector(size);
    NDArray<double> fromData(data);

    auto dft_out = dft_1d(fromData);
    auto fft_out = iterative_fft_1d(fromData);

    REQUIRE(dft_out.shape[0] == fft_out.shape[0]);
    REQUIRE(dft_out.approxEquals(fft_out));
}

// Verifies the iterative FFT on complex input (with zero imaginary part)
// matches the verified DFT applied to the equivalent real signal.
TEST_CASE("Iterative FFT (complex) matches DFT", "[fft-1d]") {
    auto size = GENERATE(1, 2, 4, 8, 256, 1024);

    auto data = generate_random_vector(size);
    NDArray<double> fromData(data);

    NDArray<std::complex<double>> complex_in = NDArray<std::complex<double>>::empty({size});
    for (size_t i = 0; i < size; i++) {
        complex_in(i) = fromData(i);
    }

    auto dft_out = dft_1d(fromData);
    auto fft_out = iterative_fft_1d(complex_in);

    REQUIRE(dft_out.shape[0] == fft_out.shape[0]);
    REQUIRE(dft_out.approxEquals(fft_out));
}

// Verifies the iterative IFFT against the verified IDFT. The iterative IFFT
// returns an un-normalized result, so we divide by N before comparing.
TEST_CASE("Iterative IFFT matches IDFT", "[ifft-1d]") {
    auto size = GENERATE(1, 2, 4, 8, 256, 1024);

    auto data = generate_random_vector(size);
    NDArray<double> fromData(data);

    auto freq = dft_1d(fromData);
    auto idft_out = idft_1d(freq);
    auto ifft_out = iterative_ifft_1d(freq);

    REQUIRE(idft_out.shape[0] == ifft_out.shape[0]);

    NDArray<double> ifft_normalized = NDArray<double>::empty({size});
    for (int i = 0; i < size; i++) {
        ifft_normalized(i) = ifft_out(i).real() / size;
    }

    REQUIRE(idft_out.approxEquals(ifft_normalized));
}

// Verifies the 2D FFT against the verified 2D DFT, exercising both the
// iterative and recursive 1D backends used internally by fft_2d.
TEST_CASE("2D FFT matches 2D DFT", "[fft-2d]") {
    auto useIterative = GENERATE(true, false);
    size_t size = 64;

    auto data = generate_random_vector(size);
    NDArray<double> in(data);
    in.reshape(8, 8);

    auto dft_out = dft_2d(in);
    auto fft_out = fft_2d(in, useIterative);

    REQUIRE(dft_out.shape[0] == fft_out.shape[0]);
    REQUIRE(dft_out.shape[1] == fft_out.shape[1]);
    REQUIRE(dft_out.approxEquals(fft_out));
}

// Verifies the 2D IFFT against the verified 2D IDFT (ifft_2d already
// performs the /N normalization internally), for both 1D backends.
TEST_CASE("2D IFFT matches 2D IDFT", "[ifft-2d]") {
    auto useIterative = GENERATE(true, false);
    size_t size = 64;

    auto data = generate_random_vector(size);
    NDArray<double> in(data);
    in.reshape(8, 8);

    auto freq = dft_2d(in);
    auto idft_out = idft_2d(freq);
    auto ifft_out = ifft_2d(freq, useIterative);

    REQUIRE(idft_out.shape[0] == ifft_out.shape[0]);
    REQUIRE(idft_out.shape[1] == ifft_out.shape[1]);
    REQUIRE(idft_out.approxEquals(ifft_out));
}