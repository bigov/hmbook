#ifndef HMB_TREE_H
#define HMB_TREE_H

#include <wx/treectrl.h>
#include "panel_view.h"
#include "tools.h"


// Класс для хранения файловых путей в элементах дерева
class FileItemData : public wxTreeItemData
{
public:
    explicit FileItemData(const wxString& filePath) : item_file_path(filePath) {}
    const wxString& GetFilePath() const { return item_file_path; }

private:
    wxString item_file_path;
};

class hmbTree : public wxTreeCtrl
{
public:
    explicit hmbTree(wxWindow* parent);
    void bind_subscriber(hmbPanelView* subscriber);
    void set_root_dir(const wxString& dir);
    void select_item(const wxString& filePath);

private:
    hmbPanelView* subscriber = nullptr; // Подписчик, который будет уведомляться при выборе файла

    void populate_tree(const wxString& path, wxTreeItemId parent);
    void on_selection(wxTreeEvent& event);
};

#endif // HMB_TREE_H
