#ifndef HMB_PANEL_VIEW_H
#define HMB_PANEL_VIEW_H

#include "wx/aui/auibook.h"
#include "wx/sizer.h"
#include "wx/textctrl.h"

#include "wx/panel.h"
#include "rich.h"

class hmbRich;
class wxAuiNotebook;
class wxTextCtrl;

class hmbPanelView : public wxPanel
{
public:
    explicit hmbPanelView(wxWindow* parent);
    void save_file_as();
    void load_file(const wxString& filePath);
    wxMenu* get_edit_menu();

    hmbRich* get_txt_rich() const;
    wxString get_current_file() const;

private:
    wxAuiNotebook* nbook = nullptr;
    hmbRich* page_rich = nullptr;
    wxTextCtrl* page_source = nullptr;
    wxTextCtrl* page_buffer = nullptr;

    void init_pages();
};

#endif // HMB_PANEL_VIEW_H