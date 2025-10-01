# Makefile for SyntaxTextCtrl Demo
# Targets wxWidgets 3.2

# Compiler
CXX = g++

# Program name
TARGET = syntax_demo

# Source files (main.cpp is local, library files are in parent directory)
SOURCES = main.cpp ../SyntaxTextCtrl.cpp
OBJECTS = $(SOURCES:.cpp=.o)

# wxWidgets configuration
WX_CONFIG = wx-config
WX_CXXFLAGS = $(shell $(WX_CONFIG) --cxxflags)
WX_LIBS = $(shell $(WX_CONFIG) --libs)

# Compiler flags
CXXFLAGS = -std=c++11 -Wall -I.. $(WX_CXXFLAGS)
LDFLAGS = $(WX_LIBS)

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS)

# Compile source files
%.o: %.cpp ../SyntaxTextCtrl.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)

# Rebuild from scratch
rebuild: clean all

# Run the application
run: $(TARGET)
	./$(TARGET)


.PHONY: all clean rebuild run install-deps check-wx
