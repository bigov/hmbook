#ifndef HMB_PANEL_VIEW_H
#define HMB_PANEL_VIEW_H

#include <wx/aui/auibook.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/panel.h>

#include "rich.h"
#include "text.h"
//#include "panel_tree.h"

class hmbPanelTree;
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
    wxMenu* edit_menu();
    void bind_subscriber(hmbStatusBar* status_bar);
    void bind_toolsbar(hmbToolsBar* tools_bar);

private:
    wxAuiNotebook* nbook = nullptr;
    hmbRich* page_rich = nullptr;
    hmbText* page_source = nullptr;
    hmbText* page_buffer = nullptr;

    void page_changed(wxAuiNotebookEvent& event);
    void init_pages();
};

#endif // HMB_PANEL_VIEW_H