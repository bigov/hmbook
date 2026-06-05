#include "status_bar.h"

#if wxUSE_STATUSBAR
hmbStatusBar::hmbStatusBar(wxWindow* parent)
    : wxStatusBar(parent, wxID_ANY)
{
    this->SetFieldsCount(2);
    int widths[] = { -1, 220 };
    this->SetStatusWidths(2, widths);
}

void hmbStatusBar::set_text_1(const wxString& text)
{
    auto t = wxString(" ") + text;
    this->SetStatusText(t, 0);
}

void hmbStatusBar::set_text_2(const wxString& text)
{
    this->SetStatusText(text, 1);
}

void hmbStatusBar::show_url(const wxString& url)
{
    this->set_text_1(url);
}

#endif // wxUSE_STATUSBAR