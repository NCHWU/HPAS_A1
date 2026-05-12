#include <iostream>
#include <catch2/catch_all.hpp>
#include <random>
#include <chrono>
#include <algorithm>
#include <vector>

#include <ft/ft.h>

//#define RUN_BENCHMARKS // Uncomment if you would like to run the example benchmark
//#define FFT_2D_TEST_OUT
#define RUN_FFT_SCALING // Q5 scaling experiment

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

// Compare recursive IFFT to IDFT. IFFT isn't normalized, so divide by N.
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

// Iterative FFT on real input should match the DFT.
TEST_CASE("Iterative FFT (real) matches DFT", "[fft-1d]") {
    auto size = GENERATE(1, 2, 4, 8, 256, 1024);

    auto data = generate_random_vector(size);
    NDArray<double> fromData(data);

    auto dft_out = dft_1d(fromData);
    auto fft_out = iterative_fft_1d(fromData);

    REQUIRE(dft_out.shape[0] == fft_out.shape[0]);
    REQUIRE(dft_out.approxEquals(fft_out));
}

// Same as above but with a complex input (zero imaginary part).
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

// Compare iterative IFFT to IDFT, same /N normalization as the recursive case.
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

// 2D FFT vs 2D DFT, run with both 1D backends.
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

// 2D IFFT vs 2D IDFT. ifft_2d already normalizes internally.
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

#ifdef RUN_FFT_SCALING
// Q5: time recursive vs iterative FFT across N = 2^4..2^20 and print
// time / (N log N) to check the asymptote. Run with:
//   ./build/HPAS_A1_Tests "[fft-scaling]" -s > scaling.csv
TEST_CASE("FFT scaling: recursive vs iterative", "[fft-scaling]") {
    constexpr int MIN_LOG2 = 4;
    constexpr int MAX_LOG2 = 20;
    constexpr int REPS = 7;

    volatile double sink = 0.0;

    std::cout << "N,log2N,recursive_ns,iterative_ns,iter_speedup,"
              << "recursive_per_NlogN,iterative_per_NlogN\n";

    for (int k = MIN_LOG2; k <= MAX_LOG2; k++) {
        const size_t N = static_cast<size_t>(1) << k;

        auto data = generate_random_vector(N);
        NDArray<double> in(data);

        std::vector<double> rec_times;
        std::vector<double> itr_times;
        rec_times.reserve(REPS);
        itr_times.reserve(REPS);

        // recursive gets too slow past 2^18, skip it for larger N
        const bool run_recursive = (k <= 18);

        for (int r = 0; r < REPS; r++) {
            if (run_recursive) {
                auto t0 = std::chrono::steady_clock::now();
                auto out = cooley_tukey_fft_1d(in);
                auto t1 = std::chrono::steady_clock::now();
                sink += out(0).real();
                rec_times.push_back(
                    std::chrono::duration<double, std::nano>(t1 - t0).count());
            }

            {
                auto t0 = std::chrono::steady_clock::now();
                auto out = iterative_fft_1d(in);
                auto t1 = std::chrono::steady_clock::now();
                sink += out(0).real();
                itr_times.push_back(
                    std::chrono::duration<double, std::nano>(t1 - t0).count());
            }
        }

        auto median = [](std::vector<double>& v) {
            std::sort(v.begin(), v.end());
            return v[v.size() / 2];
        };

        double rec_ns = run_recursive ? median(rec_times) : 0.0;
        double itr_ns = median(itr_times);
        double NlogN = static_cast<double>(N) * k;

        std::cout << N << "," << k << ",";
        if (run_recursive) std::cout << rec_ns;
        else std::cout << "NA";
        std::cout << "," << itr_ns << ",";
        if (run_recursive) std::cout << (rec_ns / itr_ns);
        else std::cout << "NA";
        std::cout << ",";
        if (run_recursive) std::cout << (rec_ns / NlogN);
        else std::cout << "NA";
        std::cout << "," << (itr_ns / NlogN) << "\n";
    }

    // print the sink so the compiler can't elide the FFT calls
    std::cout << "# sink=" << sink << "\n";

    SUCCEED("scaling experiment complete; see CSV above");
}
#endif