# SyntaxTextCtrl

A custom wxWidgets single text control with syntax highlighting and auto-completion capabilities.

wxWidget does not support syntax highlighting and completion in single text
control, hence the reason for this to exsist.

## Features

- **Syntax Highlighting**: Customizable regex-based syntax highlighting
- **Auto-completion**: Code completion based on a callback
- **Standard Text Editing**: Full support for undo/redo, copy/paste, selection, etc.

Note that this component is written using wxWidgets only, it is not backed by
any native component and hence integrates poorly with any native features such
as spell check and accessibility

## Building

### Using CMake (Recommended)

```bash
mkdir build
cd build
cmake ..
make
```


## Using with FetchContent


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
## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
