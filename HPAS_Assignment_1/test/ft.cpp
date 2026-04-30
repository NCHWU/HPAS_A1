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
