#ifndef HMB_TREE_H
#define HMB_TREE_H

#include "wx/treectrl.h"

#include "subscriber.h"

// Класс для хранения пути файла в элементе дерева
class FileItemData : public wxTreeItemData
{
public:
    explicit FileItemData(const wxString& filePath) : m_filePath(filePath) {}
    const wxString& GetFilePath() const { return m_filePath; }

private:
    wxString m_filePath;
};

class hmbTree : public wxTreeCtrl
{
public:
    wxString current_file = wxEmptyString;
    explicit hmbTree(wxWindow* parent);
    void bind_file_selected_handler(hmbSubscriber* subscriber);
    void load_directory(const wxString& dir);
    wxString get_current_dir() const;

private:
    hmbSubscriber* file_selected_subscriber = nullptr;
    wxString current_dir = wxEmptyString;

    void populate_tree(const wxString& path, wxTreeItemId parent);
    void on_selection(wxTreeEvent& event);
};

#endif // HMB_TREE_H
