# Root Makefile for SyntaxTextCtrl Library
# This Makefile builds the demo application

# Default target
all: demo

# Build the demo application
demo:
	@echo "Building demo application..."
	@cd demo && $(MAKE) all

# Clean all build artifacts
clean:
	@echo "Cleaning demo build artifacts..."
	@cd demo && $(MAKE) clean

# Rebuild everything from scratch
rebuild: clean demo

# Run the demo application
run: demo
	@echo "Running demo application..."
	@cd demo && $(MAKE) run

# Check wxWidgets installation
check-wx:
	@echo "Checking wxWidgets installation..."
	@cd demo && $(MAKE) check-wx

.PHONY: all demo clean rebuild run check-wx install-deps
