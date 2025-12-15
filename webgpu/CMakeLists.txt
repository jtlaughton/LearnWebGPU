cmake_minimum_required(VERSION 3.15)
project(webgpu-backend)

message(STATUS "Using Dawn from submodule")

# Check if Dawn submodule exists
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../external/dawn/CMakeLists.txt")
	message(FATAL_ERROR
			"Dawn submodule not found. Please run:\n"
			"  git submodule update --init --recursive\n"
			"from your project root directory."
	)
endif()

# Configure Dawn options BEFORE adding subdirectory
set(DAWN_FETCH_DEPENDENCIES ON CACHE BOOL "Fetch Dawn dependencies" FORCE)

# Backend selection - only Vulkan for Linux
if(APPLE)
	set(DAWN_ENABLE_METAL ON CACHE BOOL "" FORCE)
	set(DAWN_ENABLE_VULKAN OFF CACHE BOOL "" FORCE)
else()
	set(DAWN_ENABLE_METAL OFF CACHE BOOL "" FORCE)
	set(DAWN_ENABLE_VULKAN ON CACHE BOOL "" FORCE)
endif()

# Disable other backends
set(DAWN_ENABLE_D3D11 OFF CACHE BOOL "" FORCE)
set(DAWN_ENABLE_D3D12 OFF CACHE BOOL "" FORCE)
set(DAWN_ENABLE_NULL OFF CACHE BOOL "" FORCE)
set(DAWN_ENABLE_DESKTOP_GL OFF CACHE BOOL "" FORCE)
set(DAWN_ENABLE_OPENGLES OFF CACHE BOOL "" FORCE)

# Disable samples and tests
set(DAWN_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(TINT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(TINT_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(TINT_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)

# Enable Wayland support for Linux
if(NOT APPLE)
	set(DAWN_USE_WAYLAND ON CACHE BOOL "" FORCE)
	set(DAWN_USE_X11 ON CACHE BOOL "" FORCE)
endif()

# Add Dawn
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../external/dawn ${CMAKE_BINARY_DIR}/dawn EXCLUDE_FROM_ALL)

# Create clean webgpu interface target
add_library(webgpu INTERFACE)
target_link_libraries(webgpu INTERFACE webgpu_dawn)
target_include_directories(webgpu INTERFACE
		"${CMAKE_CURRENT_SOURCE_DIR}/../external/dawn/include"
		"${CMAKE_BINARY_DIR}/dawn/gen/include"
)

# Define WEBGPU_BACKEND_DAWN for conditional compilation
target_compile_definitions(webgpu INTERFACE WEBGPU_BACKEND_DAWN)

message(STATUS "Dawn WebGPU backend configured successfully")

# No-op function for compatibility
function(target_copy_webgpu_binaries Target)
	# Dawn is statically linked
endfunction()