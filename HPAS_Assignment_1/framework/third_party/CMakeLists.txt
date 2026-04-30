set(CMAKE_CXX_FLAGS_COPY ${CMAKE_CXX_FLAGS})
if (MSVC)
	# Disable deprecation warnings when compiling with MSVC.
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS")
    add_compile_options(/W3) # Use /W4 for all targets
endif()

# Function to suppress warnings for third-party libraries
function(suppress_warnings target_name)
	if (MSVC)
		target_compile_options(${target_name} PRIVATE /wd4267 /wd4244 /wd4996) # Add more warnings here as needed
	elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
		target_compile_options(${target_name} PRIVATE -w) # Suppress all warnings
	endif()
endfunction()

add_subdirectory("glm")
add_subdirectory("glad")
add_subdirectory("glfw3")
add_subdirectory("imgui")
add_subdirectory("tinyfiledialogs")

# Credit to https://stackoverflow.com/questions/72586514/freetype-dependencies-on-linux
# Disable all FreeType external dependencies
option(FT_DISABLE_ZLIB "Disable use of system zlib and use internal zlib library instead." ON)
option(FT_DISABLE_BZIP2 "Disable support of bzip2 compressed fonts." ON)
option(FT_DISABLE_PNG "Disable support of PNG compressed OpenType embedded bitmaps." ON)
option(FT_DISABLE_HARFBUZZ "Disable HarfBuzz (used for improving auto-hinting of OpenType fonts)." ON)
option(FT_DISABLE_BROTLI "Disable support of compressed WOFF2 fonts." ON)

add_subdirectory("freetype")

add_subdirectory("catch2")

# Apply suppression to specific targets
suppress_warnings(imgui)
suppress_warnings(tinyfiledialogs)
suppress_warnings(freetype)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS_COPY})
