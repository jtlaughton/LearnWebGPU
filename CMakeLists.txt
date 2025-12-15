cmake_minimum_required(VERSION 3.0...3.25)
project(
        LearnWebGPU # name of the project, which will also be the name of the visual studio solution if you use it
        VERSION 0.1.0 # any version number
        LANGUAGES CXX C # programming languages used by the project
)

add_subdirectory(external/glfw)
add_subdirectory(external/glfwwebgpu)
add_subdirectory(webgpu)

# We specify that we want to create a target of type executable, called "App"
add_executable(App
        src/main.cpp
        src/wgpu_utils.cpp
        src/Application.cpp
        include/Application.h)

target_include_directories(App PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(App PRIVATE glfw webgpu glfw3webgpu)

target_copy_webgpu_binaries(App)

# Set some recommended properties to configure the version of C++ and treat warnings as errors
set_target_properties(App PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        COMPILE_WARNING_AS_ERROR ON
)

# Also add more warnings, to minimize mistakes
if (MSVC)
    target_compile_options(App PRIVATE /W4)
else()
    target_compile_options(App PRIVATE -Wall -Wextra -pedantic)
endif()

# Generate "schemes" for XCode (macOS) and enable shader debugging (will become handy later on)
if (XCODE)
    set_target_properties(App PROPERTIES
            XCODE_GENERATE_SCHEME ON
            XCODE_SCHEME_ENABLE_GPU_FRAME_CAPTURE_MODE "Metal"
    )
endif()