cmake_minimum_required(VERSION 3.15)

project(NGINLib VERSION 0.1 LANGUAGES CXX)

# Enable generation of compile_commands.json (optional but helpful)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options for configuring the build
option(NGINLIB_BUILD_TESTS "Build NGINLib tests" OFF)
option(NGINLIB_BUILD_EXAMPLES "Build NGINLib examples" ON)
option(NGINLIB_DEVELOPMENT_MODE "Enable Development Mode with dummy source file" ON)

# Create the library target
if(NGINLIB_DEVELOPMENT_MODE)
    # Development mode with dummy source file
    add_library(NGINLib OBJECT NGINLibDummy.cpp)
    message(STATUS "NGINLib: Development Mode ON")

    # Set the C++ standard on the target
    set_target_properties(NGINLib PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )

    # Specify the include directories
    target_include_directories(NGINLib PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

    # Platform-specific compile definitions
    if(WIN32)
        target_compile_definitions(NGINLib PUBLIC NGIN_PLATFORM_WINDOWS)
    elseif(APPLE)
        target_compile_definitions(NGINLib PUBLIC NGIN_PLATFORM_MACOS)
    elseif(UNIX)
        target_compile_definitions(NGINLib PUBLIC NGIN_PLATFORM_LINUX)
    endif()

else()
    # INTERFACE library
    add_library(NGINLib INTERFACE)
    message(STATUS "NGINLib: Development Mode OFF (INTERFACE library)")

    # Set the C++ standard on the target
    set_target_properties(NGINLib PROPERTIES
        INTERFACE_CXX_STANDARD 23
        INTERFACE_CXX_STANDARD_REQUIRED ON
        INTERFACE_CXX_EXTENSIONS OFF
    )

    # Specify the include directories
    target_include_directories(NGINLib INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

    # Platform-specific compile definitions
    if(WIN32)
        target_compile_definitions(NGINLib INTERFACE NGIN_PLATFORM_WINDOWS)
    elseif(APPLE)
        target_compile_definitions(NGINLib INTERFACE NGIN_PLATFORM_MACOS)
    elseif(UNIX)
        target_compile_definitions(NGINLib INTERFACE NGIN_PLATFORM_LINUX)
    endif()

endif()

add_library(NGIN::NGINLib ALIAS NGINLib)

# Installation settings
include(GNUInstallDirs)

# Install the headers
install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install the library target
install(
    TARGETS NGINLib
    EXPORT NGINLibTargets
)

# Install the export set
install(
    EXPORT NGINLibTargets
    FILE NGINLibTargets.cmake
    NAMESPACE NGIN::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/NGINLib
)

# Generate and install the package config files
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/NGINLibConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/NGINLibConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/NGINLibConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/NGINLib
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/NGINLibConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/NGINLibConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/NGINLib
)

# Optionally add tests
    enable_testing()
    add_subdirectory(tests)

# Optionally add examples
if(NGINLIB_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
