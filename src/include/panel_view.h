#ifndef HMB_PANEL_VIEW_H
#define HMB_PANEL_VIEW_H

#include <wx/aui/auibook.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/panel.h>

#include "rich.h"
#include "text.h"

class wxAuiNotebook;
class wxTextCtrl;
class hmbToolsBar;


// Панель вида реализует интерфейс подписчика,
// чтобы напрямую получать уведомления о выборе файла из panel_tree.
class hmbPanelView : public wxPanel
{
public:
    explicit hmbPanelView(wxWindow* parent);

    // Реализация абстрактного метода hmbSubscriber.
    // Вызывается panel_tree при выборе файла в дереве.
    void load_file(const wxString& filePath);
    void save_file();
    void mode_switch(wxCommandEvent& event);
    void toggle_wrap(wxCommandEvent& event);
    wxMenu* edit_menu();
    void bind_statusbar(hmbStatusBar* status_bar);
    void bind_toolsbar(hmbToolsBar* tools_bar);
    void bind_paneltree_ptr(hmbPanelTree* panel_tree);

private:
    wxAuiNotebook* nbook = nullptr;
    hmbRich* page_rich = nullptr;
    hmbText* page_text = nullptr;

    void page_changed(wxAuiNotebookEvent& event);
    void init_pages();
};

#endif // HMB_PANEL_VIEW_H