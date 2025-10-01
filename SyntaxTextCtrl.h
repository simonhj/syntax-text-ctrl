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

#ifndef SYNTAX_TEXT_CTRL_H
#define SYNTAX_TEXT_CTRL_H

#include <wx/wx.h>
#include <wx/control.h>
#include <wx/listbox.h>
#include <wx/popupwin.h>
#include <vector>
#include <string>
#include <regex>
#include <functional>
#include <deque>

using ColorFunc = std::function<wxColour(const wxString&)>;

/**
 * Lambda type for completion suggestions
 * @param text The text to complete
 * @return A vector of completion suggestions
 */
using CompletionFunc = std::function<std::vector<wxString>(const wxString&)>;

/**
 * Structure to hold syntax highlighting rules
 * @param pattern The regex pattern to match
 * @param colorFunc The function to color the matched text
 */
struct SyntaxRule {
    std::wregex pattern;
    ColorFunc colorFunc;
    
    SyntaxRule(const std::string& regexPattern, ColorFunc func)
        : pattern(wxString::FromUTF8(regexPattern.c_str()).ToStdWstring()),
          colorFunc(func) {}
};

class SyntaxTextCtrl;

class CompletionPopup : public wxPopupWindow {
public:
    CompletionPopup(wxWindow* parent, SyntaxTextCtrl* textCtrl);
    
    void SetCompletions(const std::vector<wxString>& completions);
    wxString GetSelectedCompletion() const;
    bool SelectNext();
    bool SelectPrevious();
    int GetSelection() const { return m_listBox->GetSelection(); }
    bool HasCompletions() const { return m_listBox->GetCount() > 0; }
    
private:
    wxListBox* m_listBox;
    SyntaxTextCtrl* m_textCtrl;
    
    void OnListBoxClick(wxCommandEvent& event);
    void OnListBoxDClick(wxCommandEvent& event);
    void UpdateSize();
    void AcceptAndDismiss();
};

/**
 * @class SyntaxTextCtrl
 * @brief Custom single-line text input control with syntax highlighting and auto-completion for wxWidgets.
 * 
 * This is implemented using wxWidgets and its not backed by any native widgets. Hence it will integrate poorly 
 * with any native OS capabilities such as spell checking, keybindings or other accessibility features.
 * 
 * The widgets tries to play nice with clipboard, but thats about it.
 * 
 * The following features are supported:
 * - Standard keybindings for undo/redo, copy/paste, select all
 * - Syntax highlighting using regex patterns and callback function
 * - Completion suggestions using callback function
*/    
class SyntaxTextCtrl : public wxControl {
    friend class CompletionPopup;
    
public:
    SyntaxTextCtrl(wxWindow* parent, wxWindowID id = wxID_ANY,
                   const wxString& value = wxEmptyString,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   long style = 0);
    
    virtual ~SyntaxTextCtrl();
    
    void SetValue(const wxString& value);
    wxString GetValue() const { return m_text; }
    
    void AddSyntaxRule(const std::string& regexPattern, ColorFunc colorFunc);
    void ClearSyntaxRules();
    
    void SetCompletionFunction(CompletionFunc func);
    
    void SetSelection(long from, long to);
    void GetSelection(long* from, long* to) const;
    bool HasSelection() const { return m_selectionStart != m_selectionEnd; }
    
    void Undo();
    void Redo();
    bool CanUndo() const { return !m_undoStack.empty(); }
    bool CanRedo() const { return !m_redoStack.empty(); }
    
    void SetTextFont(const wxFont& font);
    void SetTextFont(int pointSize, wxFontFamily family = wxFONTFAMILY_TELETYPE,
                     wxFontStyle style = wxFONTSTYLE_NORMAL,
                     wxFontWeight weight = wxFONTWEIGHT_NORMAL);
    void SetFontSize(int pointSize);
    void SetFontFamily(wxFontFamily family);
    wxFont GetTextFont() const { return m_font; }
    int GetFontSize() const { return m_font.GetPointSize(); }
    wxFontFamily GetFontFamily() const { return m_font.GetFamily(); }
    
private:
    // Text state
    wxString m_text;
    size_t m_cursorPos;
    size_t m_selectionStart;
    size_t m_selectionEnd;
    
    // Syntax highlighting
    std::vector<SyntaxRule> m_syntaxRules;
    
    // Completion
    CompletionFunc m_completionFunc;
    CompletionPopup* m_completionPopup;
    bool m_showingCompletions;
    
    // Undo/Redo
    struct TextState {
        wxString text;
        size_t cursorPos;
    };
    std::deque<TextState> m_undoStack;
    std::deque<TextState> m_redoStack;
    static const size_t MAX_UNDO_LEVELS = 100;
    
    // Rendering
    wxFont m_font;
    wxColour m_defaultTextColor;
    wxColour m_backgroundColor;
    wxColour m_selectionColor;
    wxColour m_cursorColor;
    int m_leftMargin;
    int m_topMargin;
    
    // Cursor blinking
    wxTimer* m_cursorTimer;
    bool m_cursorVisible;
    
    int m_scrollOffset;  // Horizontal scroll position in pixels
    
    void OnPaint(wxPaintEvent& event);
    void OnChar(wxKeyEvent& event);
    void OnKeyDown(wxKeyEvent& event);
    void OnMouseDown(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnMouseUp(wxMouseEvent& event);
    void OnSetFocus(wxFocusEvent& event);
    void OnKillFocus(wxFocusEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnCursorTimer(wxTimerEvent& event);
    
    void InsertText(const wxString& text);
    void DeleteSelection();
    void DeleteChar(bool forward);
    void MoveCursor(int delta, bool select);
    void SetCursorPos(size_t pos, bool select);
    size_t GetCursorPosFromPoint(const wxPoint& point);
    wxPoint GetPointFromCursorPos(size_t pos);
    void CopyToClipboard();
    void PasteFromClipboard();
    void SelectAll();
    void SaveUndoState();
    void UpdateCompletions();
    void ShowCompletions(const std::vector<wxString>& completions);
    void HideCompletions();
    void AcceptCompletion();
    void EnsureCursorVisible();
    void UpdateControlHeight();
    
    struct ColoredSegment {
        size_t start;
        size_t length;
        wxColour color;
    };
    std::vector<ColoredSegment> GetColoredSegments() const;
    
    bool m_dragging;
    
    wxDECLARE_EVENT_TABLE();
};

#endif // SYNTAX_TEXT_CTRL_H
