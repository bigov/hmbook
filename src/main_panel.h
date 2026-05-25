#ifndef MAIN_PANEL_H
#define MAIN_PANEL_H

#include "wx/aui/auibook.h"
#include "wx/sizer.h"
#include "wx/textctrl.h"

#include "wx/panel.h"
#include "rich.h"

class hmbRich;
class wxAuiNotebook;
class wxTextCtrl;

class MainPanel : public wxPanel
{
public:
    explicit MainPanel(wxWindow* parent);
    hmbRich* get_txt_rich() const;

private:
    wxAuiNotebook* nbook;
    hmbRich* page_rich;
    wxTextCtrl* page_source;
    wxTextCtrl* page_buffer;

    void init_pages();
};

#endif // MAIN_PANEL_H