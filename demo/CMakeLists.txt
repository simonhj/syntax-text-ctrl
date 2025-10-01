# MIT License
# Copyright (c) 2024 SyntaxTextCtrl Contributors
# See LICENSE file for full license text

cmake_minimum_required(VERSION 3.16)
project(SyntaxTextCtrlDemo LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create the demo executable
add_executable(syntax_demo main.cpp)

# Link with the SyntaxTextCtrl library
target_link_libraries(syntax_demo SyntaxTextCtrl)

# Set output directory
set_target_properties(syntax_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Install the demo
install(TARGETS syntax_demo
    RUNTIME DESTINATION bin
)
