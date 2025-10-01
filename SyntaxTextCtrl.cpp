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

#include "SyntaxTextCtrl.h"
#include <wx/dcbuffer.h>
#include <wx/clipbrd.h>
#include <algorithm>

static const int CURSOR_TIMER_ID = wxID_HIGHEST + 1;

wxBEGIN_EVENT_TABLE(SyntaxTextCtrl, wxControl)
    EVT_PAINT(SyntaxTextCtrl::OnPaint)
    EVT_CHAR(SyntaxTextCtrl::OnChar)
    EVT_KEY_DOWN(SyntaxTextCtrl::OnKeyDown)
    EVT_LEFT_DOWN(SyntaxTextCtrl::OnMouseDown)
    EVT_MOTION(SyntaxTextCtrl::OnMouseMove)
    EVT_LEFT_UP(SyntaxTextCtrl::OnMouseUp)
    EVT_SET_FOCUS(SyntaxTextCtrl::OnSetFocus)
    EVT_KILL_FOCUS(SyntaxTextCtrl::OnKillFocus)
    EVT_SIZE(SyntaxTextCtrl::OnSize)
    EVT_TIMER(CURSOR_TIMER_ID, SyntaxTextCtrl::OnCursorTimer)
wxEND_EVENT_TABLE()

CompletionPopup::CompletionPopup(wxWindow* parent, SyntaxTextCtrl* textCtrl)
    : wxPopupWindow(parent, wxBORDER_SIMPLE),
      m_textCtrl(textCtrl) {
    m_listBox = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(m_listBox, 1, wxEXPAND);
    SetSizer(sizer);
    
    m_listBox->Bind(wxEVT_LISTBOX, &CompletionPopup::OnListBoxClick, this);
    m_listBox->Bind(wxEVT_LISTBOX_DCLICK, &CompletionPopup::OnListBoxDClick, this);
}

void CompletionPopup::SetCompletions(const std::vector<wxString>& completions) {
    m_listBox->Clear();
    for (const auto& comp : completions) {
        m_listBox->Append(comp);
    }
    if (m_listBox->GetCount() > 0) {
        m_listBox->SetSelection(0);
    }
    UpdateSize();
}

wxString CompletionPopup::GetSelectedCompletion() const {
    int sel = m_listBox->GetSelection();
    if (sel != wxNOT_FOUND) {
        return m_listBox->GetString(sel);
    }
    return wxEmptyString;
}

bool CompletionPopup::SelectNext() {
    int sel = m_listBox->GetSelection();
    if (sel < (int)m_listBox->GetCount() - 1) {
        m_listBox->SetSelection(sel + 1);
        return true;
    }
    return false;
}

bool CompletionPopup::SelectPrevious() {
    int sel = m_listBox->GetSelection();
    if (sel > 0) {
        m_listBox->SetSelection(sel - 1);
        return true;
    }
    return false;
}

void CompletionPopup::OnListBoxClick(wxCommandEvent& WXUNUSED(event)) {
}

void CompletionPopup::OnListBoxDClick(wxCommandEvent& WXUNUSED(event)) {
    AcceptAndDismiss();
}

void CompletionPopup::AcceptAndDismiss() {
    if (m_textCtrl) {
        m_textCtrl->AcceptCompletion();
    }
}

void CompletionPopup::UpdateSize() {
    if (m_listBox->GetCount() == 0) {
        SetClientSize(wxSize(200, 50));
        return;
    }
    
    wxClientDC dc(m_listBox);
    dc.SetFont(m_listBox->GetFont());
    int maxWidth = 150;
    
    for (unsigned int i = 0; i < m_listBox->GetCount(); i++) {
        wxSize textSize = dc.GetTextExtent(m_listBox->GetString(i));
        maxWidth = wxMax(maxWidth, textSize.GetWidth() + 30);
    }
    
    maxWidth = wxMin(maxWidth, 400);
    
    int itemHeight = m_listBox->GetCharHeight() + 4;
    int visibleItems = wxMin((int)m_listBox->GetCount(), 8);
    int height = itemHeight * visibleItems + 10;
    
    height = wxMax(height, 50);
    height = wxMin(height, 200);
    
    SetClientSize(wxSize(maxWidth, height));
    Layout();
}

SyntaxTextCtrl::SyntaxTextCtrl(wxWindow* parent, wxWindowID id,
                               const wxString& value,
                               const wxPoint& pos,
                               const wxSize& size,
                               long WXUNUSED(style))
    : wxControl(parent, id, pos, size, wxBORDER_SUNKEN | wxWANTS_CHARS),
      m_text(value),
      m_cursorPos(value.length()),
      m_selectionStart(value.length()),
      m_selectionEnd(value.length()),
      m_completionPopup(nullptr),
      m_showingCompletions(false),
      m_cursorTimer(nullptr),
      m_cursorVisible(true),
      m_scrollOffset(0),
      m_dragging(false) {
    
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    
    m_font = wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    m_defaultTextColor = *wxBLACK;
    m_backgroundColor = *wxWHITE;
    m_selectionColor = wxColour(173, 214, 255);
    m_cursorColor = *wxBLACK;
    m_leftMargin = 5;
    m_topMargin = 5;
    
    m_cursorTimer = new wxTimer(this, CURSOR_TIMER_ID);
    
    SetCursor(wxCursor(wxCURSOR_IBEAM));
    
    UpdateControlHeight();
}

SyntaxTextCtrl::~SyntaxTextCtrl() {
    if (m_cursorTimer) {
        m_cursorTimer->Stop();
        delete m_cursorTimer;
    }
    if (m_completionPopup) {
        m_completionPopup->Destroy();
    }
}

void SyntaxTextCtrl::SetValue(const wxString& value) {
    SaveUndoState();
    m_text = value;
    m_cursorPos = value.length();
    m_selectionStart = m_cursorPos;
    m_selectionEnd = m_cursorPos;
    m_scrollOffset = 0;
    EnsureCursorVisible();
    Refresh();
}

void SyntaxTextCtrl::AddSyntaxRule(const std::string& regexPattern, ColorFunc colorFunc) {
    m_syntaxRules.emplace_back(regexPattern, colorFunc);
}

void SyntaxTextCtrl::ClearSyntaxRules() {
    m_syntaxRules.clear();
}

void SyntaxTextCtrl::SetCompletionFunction(CompletionFunc func) {
    m_completionFunc = func;
}

void SyntaxTextCtrl::SetTextFont(const wxFont& font) {
    m_font = font;
    UpdateControlHeight();
    EnsureCursorVisible();
    Refresh();
}

void SyntaxTextCtrl::SetTextFont(int pointSize, wxFontFamily family,
                                  wxFontStyle style, wxFontWeight weight) {
    m_font = wxFont(pointSize, family, style, weight);
    UpdateControlHeight();
    EnsureCursorVisible();
    Refresh();
}

void SyntaxTextCtrl::SetFontSize(int pointSize) {
    m_font.SetPointSize(pointSize);
    UpdateControlHeight();
    EnsureCursorVisible();
    Refresh();
}

void SyntaxTextCtrl::SetFontFamily(wxFontFamily family) {
    m_font.SetFamily(family);
    UpdateControlHeight();
    EnsureCursorVisible();
    Refresh();
}

void SyntaxTextCtrl::SetSelection(long from, long to) {
    m_selectionStart = std::max(0L, std::min(from, (long)m_text.length()));
    m_selectionEnd = std::max(0L, std::min(to, (long)m_text.length()));
    m_cursorPos = m_selectionEnd;
    Refresh();
}

void SyntaxTextCtrl::GetSelection(long* from, long* to) const {
    if (from) *from = std::min(m_selectionStart, m_selectionEnd);
    if (to) *to = std::max(m_selectionStart, m_selectionEnd);
}

void SyntaxTextCtrl::Undo() {
    if (!CanUndo()) return;
    
    m_redoStack.push_back({m_text, m_cursorPos});
    if (m_redoStack.size() > MAX_UNDO_LEVELS) {
        m_redoStack.pop_front();
    }
    
    TextState state = m_undoStack.back();
    m_undoStack.pop_back();
    m_text = state.text;
    m_cursorPos = state.cursorPos;
    m_selectionStart = m_cursorPos;
    m_selectionEnd = m_cursorPos;
    
    HideCompletions();
    Refresh();
}

void SyntaxTextCtrl::Redo() {
    if (!CanRedo()) return;
    
    m_undoStack.push_back({m_text, m_cursorPos});
    if (m_undoStack.size() > MAX_UNDO_LEVELS) {
        m_undoStack.pop_front();
    }
    
    TextState state = m_redoStack.back();
    m_redoStack.pop_back();
    m_text = state.text;
    m_cursorPos = state.cursorPos;
    m_selectionStart = m_cursorPos;
    m_selectionEnd = m_cursorPos;
    
    HideCompletions();
    Refresh();
}

void SyntaxTextCtrl::OnPaint(wxPaintEvent& WXUNUSED(event)) {
    wxAutoBufferedPaintDC dc(this);
    
    dc.SetBackground(wxBrush(m_backgroundColor));
    dc.Clear();
    
    dc.SetFont(m_font);
    
    int textY = m_topMargin;
    wxSize clientSize = GetClientSize();
    
    dc.SetClippingRegion(m_leftMargin, 0, clientSize.GetWidth() - m_leftMargin, clientSize.GetHeight());
    
    std::vector<ColoredSegment> segments = GetColoredSegments();
    
    if (HasSelection()) {
        size_t selStart = std::min(m_selectionStart, m_selectionEnd);
        size_t selEnd = std::max(m_selectionStart, m_selectionEnd);
        
        wxString beforeSel = m_text.Mid(0, selStart);
        wxString selected = m_text.Mid(selStart, selEnd - selStart);
        
        wxSize beforeSize = dc.GetTextExtent(beforeSel);
        wxSize selSize = dc.GetTextExtent(selected);
        
        dc.SetBrush(wxBrush(m_selectionColor));
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.DrawRectangle(m_leftMargin + beforeSize.GetWidth() - m_scrollOffset, textY,
                        selSize.GetWidth(), dc.GetCharHeight());
    }
    
    int currentX = m_leftMargin - m_scrollOffset;
    
    for (const auto& seg : segments) {
        wxString segText = m_text.Mid(seg.start, seg.length);
        dc.SetTextForeground(seg.color);
        dc.DrawText(segText, currentX, textY);
        currentX += dc.GetTextExtent(segText).GetWidth();
    }
    
    if (HasFocus() && !HasSelection() && m_cursorVisible) {
        wxString beforeCursor = m_text.Mid(0, m_cursorPos);
        wxSize beforeSize = dc.GetTextExtent(beforeCursor);
        
        dc.SetPen(wxPen(m_cursorColor, 2));
        int cursorX = m_leftMargin + beforeSize.GetWidth() - m_scrollOffset;
        dc.DrawLine(cursorX, textY, cursorX, textY + dc.GetCharHeight());
    }
    
    dc.DestroyClippingRegion();
}

void SyntaxTextCtrl::OnChar(wxKeyEvent& event) {
    int keyCode = event.GetKeyCode();

    if (keyCode == WXK_RETURN || keyCode == WXK_NUMPAD_ENTER) {
        if (m_showingCompletions && m_completionPopup) {
            AcceptCompletion();
        }
        return;
    }

    if (keyCode == WXK_TAB) {
        if (m_showingCompletions && m_completionPopup) {
            AcceptCompletion();
            return;
        }
        event.Skip();
        return;
    }

    if (event.CmdDown()) {
        event.Skip();
        return;
    }

    wxChar unicodeKey = event.GetUnicodeKey();
    if (unicodeKey == WXK_NONE) {
        unicodeKey = static_cast<wxChar>(keyCode);
    }

    if (unicodeKey >= WXK_SPACE) {
        SaveUndoState();
        DeleteSelection();
        wxString ch(unicodeKey);
        InsertText(ch);
        UpdateCompletions();
        m_cursorVisible = true;
        m_cursorTimer->Start(500);
    } else {
        event.Skip();
    }
}

void SyntaxTextCtrl::OnKeyDown(wxKeyEvent& event) {
    int keyCode = event.GetKeyCode();
    bool cmdDown = event.CmdDown();
    bool ctrlDown = event.ControlDown();
    bool shiftDown = event.ShiftDown();
    bool accelDown = cmdDown || ctrlDown;
    
    if (m_showingCompletions && m_completionPopup) {
        if (keyCode == WXK_UP) {
            m_completionPopup->SelectPrevious();
            return;
        } else if (keyCode == WXK_DOWN) {
            m_completionPopup->SelectNext();
            return;
        } else if (keyCode == WXK_ESCAPE) {
            HideCompletions();
            return;
        }
    }
    
    if (accelDown && keyCode == 'Z' && !shiftDown) {
        Undo();
        return;
    }
    
    if ((accelDown && keyCode == 'Y') || (accelDown && shiftDown && keyCode == 'Z')) {
        Redo();
        return;
    }
    
    if (accelDown && keyCode == 'C') {
        CopyToClipboard();
        return;
    }
    
    if (accelDown && keyCode == 'V') {
        PasteFromClipboard();
        return;
    }
    
    if (accelDown && keyCode == 'A') {
        SelectAll();
        return;
    }
    
    if (keyCode == WXK_BACK) {
        if (HasSelection()) {
            SaveUndoState();
            DeleteSelection();
            UpdateCompletions();
        } else if (m_cursorPos > 0) {
            SaveUndoState();
            DeleteChar(false);
            UpdateCompletions();
        }
        return;
    }
    
    if (keyCode == WXK_DELETE) {
        if (HasSelection()) {
            SaveUndoState();
            DeleteSelection();
            UpdateCompletions();
        } else if (m_cursorPos < m_text.length()) {
            SaveUndoState();
            DeleteChar(true);
            UpdateCompletions();
        }
        return;
    }
    
    if (keyCode == WXK_LEFT) {
        if (ctrlDown) {
            while (m_cursorPos > 0 && m_text[m_cursorPos - 1] == ' ') {
                MoveCursor(-1, shiftDown);
            }
            while (m_cursorPos > 0 && m_text[m_cursorPos - 1] != ' ') {
                MoveCursor(-1, shiftDown);
            }
        } else {
            MoveCursor(-1, shiftDown);
        }
        HideCompletions();
        return;
    }
    
    if (keyCode == WXK_RIGHT) {
        if (ctrlDown) {
            while (m_cursorPos < m_text.length() && m_text[m_cursorPos] != ' ') {
                MoveCursor(1, shiftDown);
            }
            while (m_cursorPos < m_text.length() && m_text[m_cursorPos] == ' ') {
                MoveCursor(1, shiftDown);
            }
        } else {
            MoveCursor(1, shiftDown);
        }
        HideCompletions();
        return;
    }
    
    if (keyCode == WXK_HOME) {
        SetCursorPos(0, shiftDown);
        HideCompletions();
        return;
    }
    
    if (keyCode == WXK_END) {
        SetCursorPos(m_text.length(), shiftDown);
        HideCompletions();
        return;
    }
    
    event.Skip();
}

void SyntaxTextCtrl::OnMouseDown(wxMouseEvent& event) {
    SetFocus();
    
    size_t pos = GetCursorPosFromPoint(event.GetPosition());
    m_cursorPos = pos;
    m_selectionStart = pos;
    m_selectionEnd = pos;
    m_dragging = true;
    
    HideCompletions();
    Refresh();
}

void SyntaxTextCtrl::OnMouseMove(wxMouseEvent& event) {
    if (m_dragging && event.LeftIsDown()) {
        size_t pos = GetCursorPosFromPoint(event.GetPosition());
        m_cursorPos = pos;
        m_selectionEnd = pos;
        Refresh();
    }
}

void SyntaxTextCtrl::OnMouseUp(wxMouseEvent& WXUNUSED(event)) {
    m_dragging = false;
}

void SyntaxTextCtrl::OnSetFocus(wxFocusEvent& WXUNUSED(event)) {
    m_cursorVisible = true;
    m_cursorTimer->Start(500);
    Refresh();
}

void SyntaxTextCtrl::OnKillFocus(wxFocusEvent& WXUNUSED(event)) {
    m_cursorTimer->Stop();
    HideCompletions();
    Refresh();
}

void SyntaxTextCtrl::OnSize(wxSizeEvent& event) {
    Refresh();
    event.Skip();
}

void SyntaxTextCtrl::OnCursorTimer(wxTimerEvent& WXUNUSED(event)) {
    m_cursorVisible = !m_cursorVisible;
    Refresh();
}

void SyntaxTextCtrl::InsertText(const wxString& text) {
    m_text.insert(m_cursorPos, text);
    m_cursorPos += text.length();
    m_selectionStart = m_cursorPos;
    m_selectionEnd = m_cursorPos;
    m_redoStack.clear();
    EnsureCursorVisible();
    Refresh();
}

void SyntaxTextCtrl::DeleteSelection() {
    if (!HasSelection()) return;
    
    size_t start = std::min(m_selectionStart, m_selectionEnd);
    size_t end = std::max(m_selectionStart, m_selectionEnd);
    
    m_text.erase(start, end - start);
    m_cursorPos = start;
    m_selectionStart = start;
    m_selectionEnd = start;
    m_redoStack.clear();
    Refresh();
}

void SyntaxTextCtrl::DeleteChar(bool forward) {
    if (forward && m_cursorPos < m_text.length()) {
        m_text.erase(m_cursorPos, 1);
    } else if (!forward && m_cursorPos > 0) {
        m_text.erase(m_cursorPos - 1, 1);
        m_cursorPos--;
    }
    
    m_selectionStart = m_cursorPos;
    m_selectionEnd = m_cursorPos;
    m_redoStack.clear();
    EnsureCursorVisible();
    Refresh();
}

void SyntaxTextCtrl::MoveCursor(int delta, bool select) {
    if (!select && HasSelection() && delta != 0) {
        if (delta < 0) {
            m_cursorPos = std::min(m_selectionStart, m_selectionEnd);
        } else {
            m_cursorPos = std::max(m_selectionStart, m_selectionEnd);
        }
        m_selectionStart = m_cursorPos;
        m_selectionEnd = m_cursorPos;
    } else {
        int newPos = (int)m_cursorPos + delta;
        newPos = std::max(0, std::min(newPos, (int)m_text.length()));
        m_cursorPos = newPos;
        
        if (!select) {
            m_selectionStart = m_cursorPos;
            m_selectionEnd = m_cursorPos;
        } else {
            m_selectionEnd = m_cursorPos;
        }
    }
    
    m_cursorVisible = true;
    if (m_cursorTimer->IsRunning()) {
        m_cursorTimer->Start(500);
    }
    
    EnsureCursorVisible();
    Refresh();
}

void SyntaxTextCtrl::SetCursorPos(size_t pos, bool select) {
    m_cursorPos = std::min(pos, m_text.length());
    
    if (!select) {
        m_selectionStart = m_cursorPos;
        m_selectionEnd = m_cursorPos;
    } else {
        m_selectionEnd = m_cursorPos;
    }
    
    m_cursorVisible = true;
    if (m_cursorTimer->IsRunning()) {
        m_cursorTimer->Start(500);
    }
    
    EnsureCursorVisible();
    Refresh();
}

size_t SyntaxTextCtrl::GetCursorPosFromPoint(const wxPoint& point) {
    wxClientDC dc(this);
    dc.SetFont(m_font);
    
    int targetX = point.x - m_leftMargin + m_scrollOffset;
    if (targetX <= 0) return 0;
    
    int currentX = 0;
    for (size_t i = 0; i <= m_text.length(); i++) {
        wxString substr = m_text.Mid(0, i);
        int width = dc.GetTextExtent(substr).GetWidth();
        
        if (i == m_text.length() || width > targetX) {
            if (i > 0 && width - targetX > targetX - currentX) {
                return i - 1;
            }
            return i;
        }
        currentX = width;
    }
    
    return m_text.length();
}

wxPoint SyntaxTextCtrl::GetPointFromCursorPos(size_t pos) {
    wxClientDC dc(this);
    dc.SetFont(m_font);
    
    wxString beforeCursor = m_text.Mid(0, pos);
    int width = dc.GetTextExtent(beforeCursor).GetWidth();
    
    return wxPoint(m_leftMargin + width - m_scrollOffset, m_topMargin);
}

void SyntaxTextCtrl::CopyToClipboard() {
    if (!HasSelection()) return;
    
    size_t start = std::min(m_selectionStart, m_selectionEnd);
    size_t end = std::max(m_selectionStart, m_selectionEnd);
    wxString selected = m_text.Mid(start, end - start);
    
    if (wxTheClipboard->Open()) {
        wxTheClipboard->SetData(new wxTextDataObject(selected));
        wxTheClipboard->Close();
    }
}

void SyntaxTextCtrl::PasteFromClipboard() {
    if (wxTheClipboard->Open()) {
        if (wxTheClipboard->IsSupported(wxDF_TEXT)) {
            wxTextDataObject data;
            wxTheClipboard->GetData(data);
            
            SaveUndoState();
            DeleteSelection();
            InsertText(data.GetText());
            UpdateCompletions();
        }
        wxTheClipboard->Close();
    }
}

void SyntaxTextCtrl::SelectAll() {
    m_selectionStart = 0;
    m_selectionEnd = m_text.length();
    m_cursorPos = m_text.length();
    Refresh();
}

void SyntaxTextCtrl::SaveUndoState() {
    m_undoStack.push_back({m_text, m_cursorPos});
    if (m_undoStack.size() > MAX_UNDO_LEVELS) {
        m_undoStack.pop_front();
    }
    m_redoStack.clear();
}

void SyntaxTextCtrl::UpdateCompletions() {
    if (!m_completionFunc) return;
    
    wxString textToCursor = m_text.Mid(0, m_cursorPos);
    
    std::vector<wxString> completions = m_completionFunc(textToCursor);
    
    if (!completions.empty()) {
        ShowCompletions(completions);
    } else {
        HideCompletions();
    }
}

void SyntaxTextCtrl::ShowCompletions(const std::vector<wxString>& completions) {
    if (completions.empty()) {
        HideCompletions();
        return;
    }
    
    if (!m_completionPopup) {
        m_completionPopup = new CompletionPopup(this, this);
    }
    
    m_completionPopup->SetCompletions(completions);
    
    wxPoint cursorPoint = GetPointFromCursorPos(m_cursorPos);
    wxPoint screenPos = ClientToScreen(cursorPoint);
    screenPos.y += GetCharHeight() + 2;
    
    m_completionPopup->Position(screenPos, wxSize(0, 0));
    
    if (!m_showingCompletions) {
        m_completionPopup->Show();
        m_showingCompletions = true;
    }
}

void SyntaxTextCtrl::HideCompletions() {
    if (m_completionPopup && m_showingCompletions) {
        m_completionPopup->Hide();
        m_showingCompletions = false;
    }
}

void SyntaxTextCtrl::AcceptCompletion() {
    if (!m_showingCompletions || !m_completionPopup) return;
    
    wxString completion = m_completionPopup->GetSelectedCompletion();
    if (!completion.IsEmpty()) {
        SaveUndoState();
        
        size_t wordStart = m_cursorPos;
        while (wordStart > 0 && m_text[wordStart - 1] != ' ') {
            wordStart--;
        }
        
        m_text.erase(wordStart, m_cursorPos - wordStart);
        m_cursorPos = wordStart;
        
        InsertText(completion);
    }
    
    HideCompletions();
    SetFocus();
    Refresh();
}

void SyntaxTextCtrl::EnsureCursorVisible() {
    wxClientDC dc(this);
    dc.SetFont(m_font);
    
    wxString beforeCursor = m_text.Mid(0, m_cursorPos);
    int cursorPixelPos = dc.GetTextExtent(beforeCursor).GetWidth();
    
    wxSize clientSize = GetClientSize();
    int visibleWidth = clientSize.GetWidth() - m_leftMargin - 10;
    
    int cursorScreenPos = cursorPixelPos - m_scrollOffset;
    
    if (cursorScreenPos > visibleWidth) {
        m_scrollOffset = cursorPixelPos - visibleWidth;
    }
    else if (cursorScreenPos < 0) {
        m_scrollOffset = cursorPixelPos;
    }
    
    if (m_scrollOffset < 0) {
        m_scrollOffset = 0;
    }
}

void SyntaxTextCtrl::UpdateControlHeight() {
    wxClientDC dc(this);
    dc.SetFont(m_font);
    
    int charHeight = dc.GetCharHeight();
    
    int desiredHeight = m_topMargin * 2 + charHeight + 4;
    
    SetMinSize(wxSize(100, desiredHeight));
    
    wxWindow* parent = GetParent();
    if (parent) {
        wxSizer* sizer = parent->GetSizer();
        if (sizer) {
            sizer->Layout();
        }
    }
}

std::vector<SyntaxTextCtrl::ColoredSegment> SyntaxTextCtrl::GetColoredSegments() const {
    std::vector<ColoredSegment> segments;
    
    if (m_text.IsEmpty()) {
        return segments;
    }
    
    std::wstring text = m_text.ToStdWstring();
    std::vector<bool> matched(text.length(), false);
    
    for (const auto& rule : m_syntaxRules) {
        std::wsregex_iterator it(text.begin(), text.end(), rule.pattern);
        std::wsregex_iterator end;
        
        for (; it != end; ++it) {
            size_t start = it->position();
            size_t length = it->length();
            
            bool alreadyMatched = false;
            for (size_t i = start; i < start + length; i++) {
                if (matched[i]) {
                    alreadyMatched = true;
                    break;
                }
            }
            
            if (!alreadyMatched) {
                wxString matchedText = m_text.Mid(start, length);
                wxColour color = rule.colorFunc(matchedText);
                segments.push_back({start, length, color});
                
                for (size_t i = start; i < start + length; i++) {
                    matched[i] = true;
                }
            }
        }
    }
    
    std::sort(segments.begin(), segments.end(),
              [](const ColoredSegment& a, const ColoredSegment& b) {
                  return a.start < b.start;
              });
    
    std::vector<ColoredSegment> result;
    size_t pos = 0;
    
    for (const auto& seg : segments) {
        if (pos < seg.start) {
            result.push_back({pos, seg.start - pos, m_defaultTextColor});
        }
        result.push_back(seg);
        pos = seg.start + seg.length;
    }
    
    if (pos < m_text.length()) {
        result.push_back({pos, m_text.length() - pos, m_defaultTextColor});
    }
    
    return result;
}
