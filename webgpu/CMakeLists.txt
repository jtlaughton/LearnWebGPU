cmake_minimum_required(VERSION 3.15)
project(webgpu-backend)

include(FetchContent)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(WGPU_PLATFORM "linux-x86_64")
elseif(WIN32)
    set(WGPU_PLATFORM "windows-x86_64")
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

set(WGPU_VERSION "v27.0.4.0")

FetchContent_Declare(
    wgpu_native
    URL "https://github.com/gfx-rs/wgpu-native/releases/download/${WGPU_VERSION}/wgpu-${WGPU_PLATFORM}-release.zip"
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../external/wgpu-native"
)
FetchContent_MakeAvailable(wgpu_native)

add_library(webgpu INTERFACE)

target_include_directories(webgpu INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/../external/wgpu-native/include"
)

if(WIN32)
    target_link_libraries(webgpu INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/../external/wgpu-native/wgpu_native.dll.lib"
    )
else()
    target_link_libraries(webgpu INTERFACE
		"${CMAKE_CURRENT_SOURCE_DIR}/../external/wgpu-native/lib/libwgpu_native.a"
		dl pthread m
	)
    find_package(Vulkan REQUIRED)
    target_link_libraries(webgpu INTERFACE Vulkan::Vulkan)
endif()

target_compile_definitions(webgpu INTERFACE WEBGPU_BACKEND_WGPU)

message(STATUS "wgpu-native backend configured (${WGPU_PLATFORM})")

function(target_copy_webgpu_binaries Target)
    if(WIN32)
        # Copy the DLL next to the executable
        add_custom_command(TARGET ${Target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/../external/wgpu-native/wgpu_native.dll"
                $<TARGET_FILE_DIR:${Target}>
        )
    endif()
endfunction()