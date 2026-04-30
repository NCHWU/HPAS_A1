cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(Assignment1HPAS C CXX)


if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/framework")
	# Create framework library and include CMake scripts (compiler warnings, sanitizers and static analyzers).
	add_subdirectory("framework")
elseif(PROJECT_IS_TOP_LEVEL)
	add_subdirectory("../../../framework/" "${CMAKE_CURRENT_BINARY_DIR}/framework/")
endif()

# Adding all files in src to be compiled with the executable
file(GLOB_RECURSE SRC_FILES "src/ft/*.cpp" "src/tsne/*.cpp" "src/UserInterface.cpp" "src/Settings.cpp")
file(GLOB_RECURSE SRC_INCLUDES "src/ft/*.h" "src/tsne/*.h" "src/UserInterface.h" "src/Settings.h")

add_library(HPAS_A1_Lib ${SRC_FILES})
target_compile_definitions(HPAS_A1_Lib PRIVATE RESOURCE_ROOT="${CMAKE_CURRENT_LIST_DIR}/")
target_compile_features(HPAS_A1_Lib PRIVATE cxx_std_20)
# Link to OpenGL, and Microsoft-GSL and/or make their header files available.
target_link_libraries(HPAS_A1_Lib PRIVATE CGFramework)
enable_sanitizers(HPAS_A1_Lib)
set_project_warnings(HPAS_A1_Lib)

# Disable shadow variable warnings on Clang and GCC
if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_compile_options("-Wno-shadow")
endif()

# Set the include directories
target_include_directories(HPAS_A1_Lib PUBLIC ${SRC_INCLUDES})

add_executable(HPAS_A1
	src/main.cpp)
target_compile_definitions(HPAS_A1 PRIVATE RESOURCE_ROOT="${CMAKE_CURRENT_LIST_DIR}/")
target_link_libraries(HPAS_A1 PRIVATE CGFramework HPAS_A1_Lib)

# Runner for the tests

file(GLOB_RECURSE TEST_FILES "test/*.cpp")
add_executable(HPAS_A1_Tests ${TEST_FILES})
target_include_directories(HPAS_A1_Tests PRIVATE "src/")
target_compile_definitions(HPAS_A1_Tests PRIVATE RESOURCE_ROOT="${CMAKE_CURRENT_LIST_DIR}/")
target_link_libraries(HPAS_A1_Tests PRIVATE CGFramework HPAS_A1_Lib Catch2::Catch2WithMain)

# Get the names of all tests
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/framework/third_party/catch2/extras")
include(CTest)
include(Catch)
catch_discover_tests(HPAS_A1_Tests)

# Copy all files in the resources folder to the build directory after every successful build.
add_custom_command(TARGET HPAS_A1 POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	"${CMAKE_CURRENT_LIST_DIR}/resources/" $<TARGET_FILE_DIR:HPAS_A1>)
