cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 20)
project(ComputerGraphics C CXX)

# Slightly modified versions of the files from the cpp-starter-project by Jason Turner:
# https://github.com/lefticus/cpp_starter_project/
include("cmake/CompilerWarnings.cmake") # Enable almost all compiler warnings and CMake option to enable -Werror.
include("cmake/Sanitizers.cmake") # CMake options to enable address, memory, UB and thread sanitizers.
include("cmake/StaticAnalyzers.cmake") # CMake options to enable clang-tidy or cpp-check.

add_subdirectory("third_party")

set(OpenGL_GL_PREFERENCE GLVND) # Prevent CMake warning about legacy fallback on Linux.
find_package(OpenGL REQUIRED)
add_library(CGFramework STATIC
	
	"include/framework/WindowHandler.h" "src/WindowHandler.cpp"
	"include/framework/FileDialogs.h" "src/FileDialogs.cpp"
	"include/framework/FileReader.h" "src/FileReader.cpp"
	"include/framework/SavedSettings.h" "src/SavedSettings.cpp"

	"include/framework/shaders/Shader.h" "src/shaders/Shader.cpp"
	"include/framework/shaders/ComputeShader.h" "src/shaders/ComputeShader.cpp"
	"include/framework/shaders/ComputeShaderInterface.h" "src/shaders/ComputeShaderInterface.cpp"
	"include/framework/shaders/AbstractShader.h" "src/shaders/AbstractShader.cpp"
	"include/framework/shaders/ShaderInterface.h" "src/shaders/ShaderInterface.cpp"

	"include/framework/datareaders/DataReader.h"
	"include/framework/datareaders/MyeloidReader.h" "src/datareaders/MyeloidReader.cpp"
	"include/framework/datareaders/MNISTReader.h" "src/datareaders/MNISTReader.cpp"
	"include/framework/datareaders/PlanariaReader.h" "src/datareaders/PlanariaReader.cpp"
	"include/framework/datareaders/NearestNeighbourCalculator.h" "src/datareaders/NearestNeighbourCalculator.cpp"


	"include/framework/Camera.h" "src/Camera.cpp"
	"include/framework/Callbacks.h" "src/Callbacks.cpp"

	"include/framework/NDArray.h" # "src/NDArray.cpp" not needed, see https://stackoverflow.com/questions/495021/why-can-templates-only-be-implemented-in-the-header-file
	"src/PlotRenderer.cpp" "include/framework/PlotRenderer.h" 
	"include/framework/Framework.h" "src/Framework.cpp"
	"include/framework/FontHandler.h" "src/FontHandler.cpp"
	"include/framework/Text.h" "src/Text.cpp"
	"include/framework/PMatrixCalculator.h" "src/PMatrixCalculator.cpp"
	"include/framework/ScrollManager.h" "src/ScrollManager.cpp"

	"include/framework/DebugRenderData.h"
)

include_directories(
    "third_party/tinyfiledialogs/"
)

# OpenMP Support; adapted from the template of DSAIT4120's assignment 0
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
	# Support of newer OpenMP features through the -opnemp:llvm compiler option.
	# https://devblogs.microsoft.com/cppblog/improved-openmp-support-for-cpp-in-visual-studio/
	# Adapted with updates from: https://cmake.org/cmake/help/latest/module/FindOpenMP.html
	# Remove any existing /openmp flag
	if (MSVC)
		# Add /openmp:llvm flag for advanced support of advanced features in advanced MSVC.
		if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.30")
			set(OpenMP_RUNTIME_MSVC "llvm")
			message(STATUS "Enabled advanced OpenMP features for MSVC via CMake's runtime specifying variable")
		else()
			string(REPLACE "/openmp" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -openmp:llvm")
			message(STATUS "Enabled advanced OpenMP features for MSVC for CMake < 3.30")
		endif()

	endif()
	target_link_libraries(CGFramework PUBLIC OpenMP::OpenMP_CXX)
else()
	message(FATAL_ERROR "OpenMP was not found!")
endif()


target_compile_definitions(CGFramework PRIVATE RESOURCE_ROOT="${CMAKE_CURRENT_LIST_DIR}/")
target_include_directories(CGFramework PRIVATE "include/framework/" PUBLIC "include/" "third_party/tinyfiledialogs/")
target_link_libraries(CGFramework PUBLIC OpenGL::GL glad glm glfw imgui tinyfiledialogs freetype)
if (MSVC)
	target_link_libraries(CGFramework PUBLIC ws2_32)
endif()
target_compile_features(CGFramework PUBLIC cxx_std_20)
set_property(TARGET CGFramework PROPERTY POSITION_INDEPENDENT_CODE ON)

# Prevent accidentally picking up a system-wide install of another loader (e.g. GLEW).
#target_compile_definitions(CGFramework PUBLIC "-DIMGUI_IMPL_OPENGL_LOADER_GLAD=1")
