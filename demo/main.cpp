/*
 * MIT License
 *
 * Copyright (c) 2024 SyntaxTextCtrl Contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <wx/wx.h>
#include "SyntaxTextCtrl.h"

// Demo application showcasing a custom mini-language
// The mini-language has:
// - Keywords: let, if, then, else, print, return, function
// - Operators: +, -, *, /, =, ==, !=, <, >, <=, >=
// - Numbers: integers and floats
// - Strings: text in quotes
// - Comments: // single line

class MyFrame : public wxFrame {
public:
    MyFrame();
    
private:
    SyntaxTextCtrl* m_textCtrl1;
    SyntaxTextCtrl* m_textCtrl2;
    SyntaxTextCtrl* m_textCtrl3;
    wxTextCtrl* m_output;
    
    void SetupSyntaxHighlighting(SyntaxTextCtrl* ctrl);
    void SetupCompletions(SyntaxTextCtrl* ctrl);
    
    // Font demo handlers
    void OnIncreaseFontSize(wxCommandEvent& event);
    void OnDecreaseFontSize(wxCommandEvent& event);
    void OnChangeToMonospace(wxCommandEvent& event);
    void OnChangeToSansSerif(wxCommandEvent& event);
};

class MyApp : public wxApp {
public:
    virtual bool OnInit();
};

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit() {
    MyFrame* frame = new MyFrame();
    frame->Show(true);
    return true;
}

MyFrame::MyFrame()
    : wxFrame(nullptr, wxID_ANY, "SyntaxTextCtrl Demo - Custom Mini-Language",
              wxDefaultPosition, wxSize(800, 600)) {
    
    wxPanel* panel = new wxPanel(this);
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // Title
    wxStaticText* title = new wxStaticText(panel, wxID_ANY,
        "Custom Single-Line Text Control with Syntax Highlighting & Completion");
    wxFont titleFont = title->GetFont();
    titleFont.SetPointSize(12);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    title->SetFont(titleFont);
    mainSizer->Add(title, 0, wxALL | wxALIGN_CENTER, 10);
    
    // Instructions
    wxStaticText* instructions = new wxStaticText(panel, wxID_ANY,
        "Try typing keywords (let, if, print, return), numbers, operators (+, -, *, /), or strings (\"text\").\n"
        "Auto-completion will appear as you type. Use arrow keys to navigate, Enter/Tab to accept.\n"
        "Supports: Ctrl+Z/Y (undo/redo), Ctrl+A/C/V (select all/copy/paste), arrow keys, Home/End, selection.");
    mainSizer->Add(instructions, 0, wxALL | wxEXPAND, 10);
    
    // Font configuration buttons
    wxBoxSizer* fontSizer = new wxBoxSizer(wxHORIZONTAL);
    fontSizer->Add(new wxStaticText(panel, wxID_ANY, "Font Controls:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    
    wxButton* btnIncrease = new wxButton(panel, wxID_ANY, "Increase Size");
    btnIncrease->Bind(wxEVT_BUTTON, &MyFrame::OnIncreaseFontSize, this);
    fontSizer->Add(btnIncrease, 0, wxRIGHT, 5);
    
    wxButton* btnDecrease = new wxButton(panel, wxID_ANY, "Decrease Size");
    btnDecrease->Bind(wxEVT_BUTTON, &MyFrame::OnDecreaseFontSize, this);
    fontSizer->Add(btnDecrease, 0, wxRIGHT, 10);
    
    wxButton* btnMonospace = new wxButton(panel, wxID_ANY, "Monospace");
    btnMonospace->Bind(wxEVT_BUTTON, &MyFrame::OnChangeToMonospace, this);
    fontSizer->Add(btnMonospace, 0, wxRIGHT, 5);
    
    wxButton* btnSansSerif = new wxButton(panel, wxID_ANY, "Sans-Serif");
    btnSansSerif->Bind(wxEVT_BUTTON, &MyFrame::OnChangeToSansSerif, this);
    fontSizer->Add(btnSansSerif, 0);
    
    mainSizer->Add(fontSizer, 0, wxALL, 10);
    
    // Example 1: Variable declaration
    mainSizer->Add(new wxStaticText(panel, wxID_ANY, "Example 1: Variable Declaration"),
                   0, wxLEFT | wxRIGHT | wxTOP, 10);
    m_textCtrl1 = new SyntaxTextCtrl(panel, wxID_ANY,
                                     "let counter = 42 + 3.14",
                                     wxDefaultPosition, wxDefaultSize);
    SetupSyntaxHighlighting(m_textCtrl1);
    SetupCompletions(m_textCtrl1);
    mainSizer->Add(m_textCtrl1, 0, wxALL | wxEXPAND, 10);
    
    // Example 2: Conditional expression
    mainSizer->Add(new wxStaticText(panel, wxID_ANY, "Example 2: Conditional Expression"),
                   0, wxLEFT | wxRIGHT | wxTOP, 10);
    m_textCtrl2 = new SyntaxTextCtrl(panel, wxID_ANY,
                                     "if x >= 10 then print \"large\"",
                                     wxDefaultPosition, wxDefaultSize);
    SetupSyntaxHighlighting(m_textCtrl2);
    SetupCompletions(m_textCtrl2);
    mainSizer->Add(m_textCtrl2, 0, wxALL | wxEXPAND, 10);
    
    // Example 3: Function definition
    mainSizer->Add(new wxStaticText(panel, wxID_ANY, "Example 3: Function Definition"),
                   0, wxLEFT | wxRIGHT | wxTOP, 10);
    m_textCtrl3 = new SyntaxTextCtrl(panel, wxID_ANY,
                                     "function add(a, b) return a + b",
                                     wxDefaultPosition, wxDefaultSize);
    SetupSyntaxHighlighting(m_textCtrl3);
    SetupCompletions(m_textCtrl3);
    mainSizer->Add(m_textCtrl3, 0, wxALL | wxEXPAND, 10);
    
    // Output area
    mainSizer->Add(new wxStaticText(panel, wxID_ANY, "Tips:"),
                   0, wxLEFT | wxRIGHT | wxTOP, 10);
    m_output = new wxTextCtrl(panel, wxID_ANY, 
        "• Blue = Keywords (let, if, then, else, print, return, function)\n"
        "• Green = Numbers (integers and floats)\n"
        "• Red = Operators (+, -, *, /, =, ==, !=, <, >, <=, >=)\n"
        "• Purple = Strings (\"text in quotes\")\n"
        "• Gray = Comments (// comment text)\n"
        "• Auto-completion suggests keywords and common functions\n"
        "• All standard text editing features supported",
        wxDefaultPosition, wxSize(-1, 150),
        wxTE_MULTILINE | wxTE_READONLY | wxTE_WORDWRAP);
    mainSizer->Add(m_output, 1, wxALL | wxEXPAND, 10);
    
    panel->SetSizer(mainSizer);
    
    Centre();
}

void MyFrame::SetupSyntaxHighlighting(SyntaxTextCtrl* ctrl) {
    // Keywords: let, if, then, else, print, return, function
    ctrl->AddSyntaxRule(
        "\\b(let|if|then|else|print|return|function)\\b",
        [](const wxString& WXUNUSED(text)) {
            return wxColour(0, 0, 255); // Blue
        }
    );
    
    // Numbers (integers and floats)
    ctrl->AddSyntaxRule(
        "\\b\\d+(\\.\\d+)?\\b",
        [](const wxString& WXUNUSED(text)) {
            return wxColour(0, 128, 0); // Green
        }
    );
    
    // Operators
    ctrl->AddSyntaxRule(
        "[+\\-*/=<>!]+",
        [](const wxString& WXUNUSED(text)) {
            return wxColour(255, 0, 0); // Red
        }
    );
    
    // Strings (text in quotes)
    ctrl->AddSyntaxRule(
        "\"[^\"]*\"",
        [](const wxString& WXUNUSED(text)) {
            return wxColour(128, 0, 128); // Purple
        }
    );
    
    // Comments (// to end of line)
    ctrl->AddSyntaxRule(
        "//.*",
        [](const wxString& WXUNUSED(text)) {
            return wxColour(128, 128, 128); // Gray
        }
    );
}

void MyFrame::SetupCompletions(SyntaxTextCtrl* ctrl) {
    ctrl->SetCompletionFunction([](const wxString& textToCursor) -> std::vector<wxString> {
        std::vector<wxString> allCompletions = {
            "let", "if", "then", "else", "print", "return", "function",
            "true", "false", "null",
            "add", "subtract", "multiply", "divide",
            "length", "concat", "split"
        };
        
        // Find the current word being typed (after last space)
        wxString currentWord;
        int lastSpace = textToCursor.Find(' ', true);
        if (lastSpace == wxNOT_FOUND) {
            currentWord = textToCursor;
        } else {
            currentWord = textToCursor.Mid(lastSpace + 1);
        }
        
        // Filter completions that start with current word
        std::vector<wxString> matches;
        if (!currentWord.IsEmpty()) {
            for (const auto& comp : allCompletions) {
                if (comp.Lower().StartsWith(currentWord.Lower())) {
                    matches.push_back(comp);
                }
            }
        }
        
        return matches;
    });
}

void MyFrame::OnIncreaseFontSize(wxCommandEvent& WXUNUSED(event)) {
    int currentSize = m_textCtrl1->GetFontSize();
    int newSize = currentSize + 2;
    if (newSize <= 24) { // Max size limit
        m_textCtrl1->SetFontSize(newSize);
        m_textCtrl2->SetFontSize(newSize);
        m_textCtrl3->SetFontSize(newSize);
    }
}

void MyFrame::OnDecreaseFontSize(wxCommandEvent& WXUNUSED(event)) {
    int currentSize = m_textCtrl1->GetFontSize();
    int newSize = currentSize - 2;
    if (newSize >= 6) { // Min size limit
        m_textCtrl1->SetFontSize(newSize);
        m_textCtrl2->SetFontSize(newSize);
        m_textCtrl3->SetFontSize(newSize);
    }
}

void MyFrame::OnChangeToMonospace(wxCommandEvent& WXUNUSED(event)) {
    m_textCtrl1->SetFontFamily(wxFONTFAMILY_TELETYPE);
    m_textCtrl2->SetFontFamily(wxFONTFAMILY_TELETYPE);
    m_textCtrl3->SetFontFamily(wxFONTFAMILY_TELETYPE);
}

void MyFrame::OnChangeToSansSerif(wxCommandEvent& WXUNUSED(event)) {
    m_textCtrl1->SetFontFamily(wxFONTFAMILY_SWISS);
    m_textCtrl2->SetFontFamily(wxFONTFAMILY_SWISS);
    m_textCtrl3->SetFontFamily(wxFONTFAMILY_SWISS);
}
