#ifndef HMB_TOOLS_BAR_H
#define HMB_TOOLS_BAR_H

#include <wx/panel.h>
#include <wx/toolbar.h>

// Идентификатор кнопки переключения переноса слов на вкладке "Text".
enum { HMB_ID_WRAP = wxID_HIGHEST + 1 };

class hmbToolsBar : public wxPanel
{
public:
    explicit hmbToolsBar(wxWindow* parent);
    void save_btn_enable(bool state);
    void text_mode_enable(bool state);
    void wrap_btn_enable(bool state);

private:
    wxToolBar* toolbar = nullptr;

    void init_toolsbar();
};

#endif // HMB_TOOLS_BAR_H