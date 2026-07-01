#ifndef HMB_TOOLS_BAR_H
#define HMB_TOOLS_BAR_H

#include <wx/panel.h>
#include <wx/toolbar.h>
#include "tools.h"

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
    void add_spacer(int width = 1, int height = 1);
};

#endif // HMB_TOOLS_BAR_H