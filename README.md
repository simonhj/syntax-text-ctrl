# SyntaxTextCtrl

A custom wxWidgets text control with syntax highlighting and auto-completion capabilities.

## Features

- **Syntax Highlighting**: Customizable regex-based syntax highlighting
- **Auto-completion**: Intelligent code completion with customizable suggestions
- **Standard Text Editing**: Full support for undo/redo, copy/paste, selection, etc.
- **Font Control**: Dynamic font size and family changes
- **wxWidgets Integration**: Built on top of wxWidgets for cross-platform compatibility

## Building

### Using CMake (Recommended)

```bash
mkdir build
cd build
cmake ..
make
```


## Using with FetchContent

To use this library in your own CMake project:

```cmake
cmake_minimum_required(VERSION 3.16)
project(MyProject LANGUAGES CXX)

# Include FetchContent
include(FetchContent)

# Fetch SyntaxTextCtrl
FetchContent_Declare(
    SyntaxTextCtrl
    GIT_REPOSITORY https://github.com/yourusername/smart_edit.git
    GIT_TAG main  # or a specific tag like v1.0.0
)

# Make it available
FetchContent_MakeAvailable(SyntaxTextCtrl)

# Create your application
add_executable(my_app main.cpp)

# Link with SyntaxTextCtrl
target_link_libraries(my_app SyntaxTextCtrl::SyntaxTextCtrl)
```

## Basic Usage

```cpp
#include <wx/wx.h>
#include "SyntaxTextCtrl.h"

// Create a SyntaxTextCtrl
SyntaxTextCtrl* textCtrl = new SyntaxTextCtrl(parent, wxID_ANY,
    "let x = 42", wxDefaultPosition, wxDefaultSize);

// Add syntax highlighting rules
textCtrl->AddSyntaxRule(
    "\\b(let|if|then|else|print|return|function)\\b",
    [](const wxString& text) { return wxColour(0, 0, 255); }  // Blue for keywords
);

textCtrl->AddSyntaxRule(
    "\\b\\d+(\\.\\d+)?\\b",
    [](const wxString& text) { return wxColour(0, 128, 0); }  // Green for numbers
);

// Set up auto-completion
textCtrl->SetCompletionFunction([](const wxString& textToCursor) -> std::vector<wxString> {
    return {"let", "if", "print", "return", "function"};
});
```

## Dependencies

- wxWidgets 3.2 or later
- C++11 compatible compiler
- CMake 3.16 or later (for CMake builds)

## Installation

### Ubuntu/Debian
```bash
sudo apt-get install libwxgtk3.2-dev
```

### macOS
```bash
brew install wxwidgets
```

### Windows
Download wxWidgets from [wxwidgets.org](https://www.wxwidgets.org/downloads/)

## License

[Add your license here]
