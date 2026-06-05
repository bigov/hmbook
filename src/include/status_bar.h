#ifndef HMB_STATUS_BAR_H
#define HMB_STATUS_BAR_H

#include <wx/statusbr.h>
#include <wx/string.h>

class hmbStatusBar : public wxStatusBar
{
public:
    explicit hmbStatusBar(wxWindow* parent);
    void set_text_1(const wxString& text);
    void set_text_2(const wxString& text);
    void show_url(const wxString& url);
};

#endif // HMB_STATUS_BAR_H