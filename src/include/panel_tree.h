#ifndef HMB_PANEL_TREE_H
#define HMB_PANEL_TREE_H

#include <wx/dirdlg.h>
#include <wx/panel.h>
#include <wx/sizer.h>

#include "panel_view.h"

class hmbTree;

class hmbPanelTree : public wxPanel
{
public:
    explicit hmbPanelTree(wxWindow* parent);
    ~hmbPanelTree();

    // Регистрация подписчика на выбор файла в дереве.
    // Подписчик должен жить не меньше, чем hmbPanelTree, пока активна подписка.
    // Передача nullptr - отключение подписки.
    void bind_subscriber(hmbPanelView* subscriber);
    void open_dir();
    void set_root_dir(const wxString& dir);
    void set_cursor_to(const wxString& current_file);

private:
    // Внутренний контрол дерева файлов.
    hmbTree* tree_viewer = nullptr;
};

#endif // HMB_PANEL_TREE_H