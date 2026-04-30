#include <iostream>
#include <catch2/catch_all.hpp>
#include <random>
#include <framework/NDArray.h>
#include <stdexcept>
#include <glm/glm.hpp>

TEST_CASE("NDArray 1 Dimension In/Out", "[ndarray]")
{
    NDArray<double> array = NDArray<double>::empty({ 5 });

    for (int i = 0; i < 5; i++)
    {
        array(i) = i + 1;
    }

    for (int i = 0; i < 5; i++)
    {
        REQUIRE(array(i) == i + 1);
    }

    // Out-of-bounds tests
    REQUIRE_THROWS_AS(array(5), std::out_of_range);
}

TEST_CASE("NDArray 2 Dimensions In/Out", "[ndarray]")
{
    NDArray<int> array = NDArray<int>::empty({ 5, 2 });

    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            array(i, j) = i*2 + j;
        }
    }

    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            REQUIRE(array(i, j) == i*2 + j);
        }
    }

    // Out-of-bounds tests
    REQUIRE_THROWS_AS(array(5, 0), std::out_of_range);
    REQUIRE_THROWS_AS(array(0, 2), std::out_of_range);

    // Incorrect number of indices
    REQUIRE_THROWS_AS(array(0), std::invalid_argument);
    REQUIRE_THROWS_AS(array(0, 0, 0), std::invalid_argument);
}

TEST_CASE("NDArray 3 Dimensions In/Out", "[ndarray]")
{
    NDArray<int> array = NDArray<int>::empty({ 3, 4, 5 });

    for (int i = 0; i < array.shape[0]; i++)
    {
        for (int j = 0; j < array.shape[1]; j++)
        {
            for (int k = 0; k < array.shape[2]; k++)
            {
                array(i, j, k) = i + j + k;
            }
        }
    }

    for (int i = 0; i < array.shape[0]; i++)
    {
        for (int j = 0; j < array.shape[1]; j++)
        {
            for (int k = 0; k < array.shape[2]; k++)
            {
                REQUIRE(array(i, j, k) == i + j + k);
            }
        }
    }

    // Out-of-bounds tests
    REQUIRE_THROWS_AS(array(3, 0, 0), std::out_of_range);
    REQUIRE_THROWS_AS(array(0, 4, 0), std::out_of_range);
    REQUIRE_THROWS_AS(array(0, 0, 5), std::out_of_range);
    REQUIRE_THROWS_AS(array(-1, 0, 0), std::out_of_range);
    REQUIRE_THROWS_AS(array(0, -1, 0), std::out_of_range);
    REQUIRE_THROWS_AS(array(0, 0, -1), std::out_of_range);

    // Incorrect number of indices
    REQUIRE_THROWS_AS(array(0, 0), std::invalid_argument);
    REQUIRE_THROWS_AS(array(0, 0, 0, 0), std::invalid_argument);
}

TEST_CASE("NDArray Initializer List", "[ndarray]")
{
    NDArray<int> array{ 1, 2, 3, 4, 5 };

    for (int i = 0; i < array.shape[0]; i++)
    {
        REQUIRE(array(i) == i + 1);
    }

    // Out-of-bounds test
    REQUIRE_THROWS_AS(array(5), std::out_of_range);
}

TEST_CASE("NDArray std::vector", "[ndarray]")
{
    std::vector<int> data{ 1, 2, 3, 4, 5 };
    NDArray<int> array(data);

    for (int i = 0; i < array.shape[0]; i++)
    {
        REQUIRE(array(i) == i + 1);
    }

    // Out-of-bounds test
    REQUIRE_THROWS_AS(array(5), std::out_of_range);
}

TEST_CASE("NDArray Copy Constructor", "[ndarray]")
{
    NDArray<int> original = NDArray<int>::empty({ 3, 3 });

    // Fill original array with values
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            original(i, j) = i * 3 + j;
        }
    }

    // Copy the array
    NDArray<int> copy = original;

    // Ensure copied values are the same
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            REQUIRE(copy(i, j) == original(i, j));
        }
    }

    // Ensure modifying the copy does NOT change the original
    copy(0, 0) = 99;
    REQUIRE(original(0, 0) != 99);
}

TEST_CASE("NDArray Copy Assignment", "[ndarray]")
{
    NDArray<int> original = NDArray<int>::empty({ 2, 2 });
    original(0, 0) = 1;
    original(0, 1) = 2;
    original(1, 0) = 3;
    original(1, 1) = 4;

    NDArray<int> copy;
    copy = original;  // Copy assignment

    REQUIRE(copy(0, 0) == 1);
    REQUIRE(copy(0, 1) == 2);
    REQUIRE(copy(1, 0) == 3);
    REQUIRE(copy(1, 1) == 4);

    // Modify copy and check that original remains unchanged
    copy(1, 1) = 99;
    REQUIRE(original(1, 1) != 99);
}

TEST_CASE("NDArray Move Constructor", "[ndarray]")
{
    NDArray<int> original = NDArray<int>::empty({ 3, 3 });

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            original(i, j) = i * 3 + j;
        }
    }

    NDArray<int> moved = std::move(original);

    // Ensure moved object retains original values
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            REQUIRE(moved(i, j) == i * 3 + j);
        }
    }

    // Ensure original is now in a moved-from state (shape should be empty)
    REQUIRE(original.shape.empty());
}

TEST_CASE("NDArray Move Assignment", "[ndarray]")
{
    NDArray<int> original = NDArray<int>::empty({ 2, 2 });
    original(0, 0) = 1;
    original(0, 1) = 2;
    original(1, 0) = 3;
    original(1, 1) = 4;

    NDArray<int> moved;
    moved = std::move(original);  // Move assignment

    REQUIRE(moved(0, 0) == 1);
    REQUIRE(moved(0, 1) == 2);
    REQUIRE(moved(1, 0) == 3);
    REQUIRE(moved(1, 1) == 4);

    // Ensure original is now in a moved-from state
    REQUIRE(original.shape.empty());
}

TEST_CASE("NDArray Clone Method", "[ndarray]")
{
    NDArray<int> original = NDArray<int>::empty({ 3, 3 });

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            original(i, j) = i * 3 + j;
        }
    }

    NDArray<int> clone = original.clone();

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            REQUIRE(clone(i, j) == original(i, j));
        }
    }

    // Ensure modifying the clone does NOT change the original
    clone(0, 0) = 99;
    REQUIRE(original(0, 0) != 99);
}

TEST_CASE("NDArray Read/Write to file 1D", "[ndarray]")
{
    NDArray<int> original = NDArray<int>::empty({ 3 });

    for (int i = 0; i < 3; i++)
    {
            original(i) = i;
    }

	original.writeToFile("test.txt");

	NDArray<int> read = NDArray<int>::readFromFile("test.txt");

    REQUIRE(read.approxEquals(original));
}

TEST_CASE("NDArray Read/Write to file 2D", "[ndarray]")
{
    NDArray<int> original = NDArray<int>::empty({ 3, 4 });

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            original(i, j) = i * 4 + j;
        }
    }

    original.writeToFile("test.txt");

    NDArray<int> read = NDArray<int>::readFromFile("test.txt");

    REQUIRE(read.approxEquals(original));
}

TEST_CASE("NDArray Read/Write to file 3D", "[ndarray]")
{
    NDArray<int> original = NDArray<int>::empty({ 3, 4, 5 });

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 4; j++)
        {
			for (int k = 0; k < 5; k++)
			{
				original(i, j, k) = i + j + k;
			}
        }
    }

    original.writeToFile("test.txt");

    NDArray<int> read = NDArray<int>::readFromFile("test.txt");

    REQUIRE(read.approxEquals(original));
}

TEST_CASE("NDArray Read/Write to file 3D double", "[ndarray]")
{
    NDArray<double> original = NDArray<double>::empty({ 3, 4, 5 });

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            for (int k = 0; k < 5; k++)
            {
                original(i, j, k) = i + j + k;
            }
        }
    }

    original.writeToFile("test.txt");

    NDArray<double> read = NDArray<double>::readFromFile("test.txt");

    REQUIRE(read.approxEquals(original));
}

TEST_CASE("NDArray Read/Write to file 3D double vec", "[ndarray]")
{
    NDArray<glm::dvec2> original = NDArray<glm::dvec2>::empty({ 3, 4, 5 });

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            for (int k = 0; k < 5; k++)
            {
                original(i, j, k) = glm::dvec2(i + j + k, i + j / (k+1));
            }
        }
    }

    original.writeToFile("test.txt");

    NDArray<glm::dvec2> read = NDArray<glm::dvec2>::readFromFile("test.txt");

    REQUIRE(read.approxEquals(original));
}