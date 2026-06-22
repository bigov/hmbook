#ifndef HMB_TOOLS_BAR_H
#define HMB_TOOLS_BAR_H

#include <wx/panel.h>
#include <wx/toolbar.h>

class hmbToolsBar : public wxPanel
{
public:
    explicit hmbToolsBar(wxWindow* parent);
    void save_btn_enable(bool state);
    void text_mode_enable(bool state);

private:
    wxToolBar* toolbar = nullptr;

    void init_toolsbar();
};

#endif // HMB_TOOLS_BAR_H