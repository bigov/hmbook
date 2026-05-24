#ifndef MAIN_PANEL_H
#define MAIN_PANEL_H

#include "wx/aui/auibook.h"
#include "wx/sizer.h"
#include "wx/textctrl.h"

#include "wx/panel.h"
#include "txt_rich.h"

class TxtRich;
class wxAuiNotebook;
class wxTextCtrl;

class MainPanel : public wxPanel
{
public:
    explicit MainPanel(wxWindow* parent);
    TxtRich* get_txt_rich() const;

private:
    wxAuiNotebook* notebook;
    TxtRich* page_rich;
    wxTextCtrl* page_source;
    wxTextCtrl* page_buffer;

    void init_pages();
};

#endif // MAIN_PANEL_H