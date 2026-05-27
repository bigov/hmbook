#ifndef HMB_PANEL_VIEW_H
#define HMB_PANEL_VIEW_H

#include "wx/aui/auibook.h"
#include "wx/sizer.h"
#include "wx/textctrl.h"
#include "wx/panel.h"

#include "rich.h"

class hmbRich;
class hmbPanelTree;
class wxAuiNotebook;
class wxTextCtrl;

// Панель вида реализует интерфейс подписчика,
// чтобы напрямую получать уведомления о выборе файла из panel_tree.
class hmbPanelView : public wxPanel
{
public:
    explicit hmbPanelView(wxWindow* parent);

    // Реализация абстрактного метода hmbSubscriber.
    // Вызывается panel_tree при выборе файла в дереве.
    void load_file(const wxString& filePath);
    void save_file_as();
    wxMenu* get_edit_menu();

private:
    wxAuiNotebook* nbook = nullptr;
    hmbRich* page_rich = nullptr;
    wxTextCtrl* page_source = nullptr;
    wxTextCtrl* page_buffer = nullptr;

    void init_pages();
};

#endif // HMB_PANEL_VIEW_H