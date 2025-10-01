# MIT License
# Copyright (c) 2024 SyntaxTextCtrl Contributors
# See LICENSE file for full license text

cmake_minimum_required(VERSION 3.16)
project(SyntaxTextCtrl VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wunused-variable -Werror")


# Find wxWidgets
find_package(wxWidgets REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

# Create the library
add_library(SyntaxTextCtrl
    SyntaxTextCtrl.cpp
    SyntaxTextCtrl.h
)

# Set target properties
set_target_properties(SyntaxTextCtrl PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER SyntaxTextCtrl.h
)

# Link wxWidgets
target_link_libraries(SyntaxTextCtrl ${wxWidgets_LIBRARIES})
target_include_directories(SyntaxTextCtrl PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Compiler features
target_compile_features(SyntaxTextCtrl PUBLIC cxx_std_11)

# Install rules
install(TARGETS SyntaxTextCtrl
    EXPORT SyntaxTextCtrlTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include
)

# Install headers
install(FILES SyntaxTextCtrl.h
    DESTINATION include
)

# Create and install config files
include(CMakePackageConfigHelpers)

# Generate the config file
write_basic_package_version_file(
    SyntaxTextCtrlConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Create config file
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/SyntaxTextCtrlConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/SyntaxTextCtrlConfig.cmake
    INSTALL_DESTINATION lib/cmake/SyntaxTextCtrl
)

# Install config files
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/SyntaxTextCtrlConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/SyntaxTextCtrlConfigVersion.cmake
    DESTINATION lib/cmake/SyntaxTextCtrl
)

# Install export
install(EXPORT SyntaxTextCtrlTargets
    FILE SyntaxTextCtrlTargets.cmake
    NAMESPACE SyntaxTextCtrl::
    DESTINATION lib/cmake/SyntaxTextCtrl
)

# Enable testing
enable_testing()

# Add demo as subdirectory (optional)
option(BUILD_DEMO "Build the demo application" ON)
if(BUILD_DEMO)
    add_subdirectory(demo)
endif()
