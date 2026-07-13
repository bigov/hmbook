#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>

#include "tree.h"
#include "tools.h"

namespace
{
wxTreeItemId find_item_by_path(wxTreeCtrl* tree, const wxTreeItemId& parent, const wxString& filePath)
{
    if (!parent.IsOk()) return wxTreeItemId();

    wxTreeItemIdValue cookie = nullptr;
    wxTreeItemId child = tree->GetFirstChild(parent, cookie);

    while (child.IsOk())
    {
        auto* data = dynamic_cast<FileItemData*>(tree->GetItemData(child));
        if (data && data->GetFilePath() == filePath)
            return child;

        wxTreeItemId nested = find_item_by_path(tree, child, filePath);
        if (nested.IsOk())
            return nested;

        child = tree->GetNextChild(parent, cookie);
    }

    return wxTreeItemId();
}
}

hmbTreeCtrl::hmbTreeCtrl(wxWindow* parent)
    : wxTreeCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                 wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT | wxTR_SINGLE | wxBORDER_NONE)
{
    // Привязываем событие выбора элемента в дереве
    Bind(wxEVT_TREE_SEL_CHANGED, &hmbTreeCtrl::on_selection, this);
}

void hmbTreeCtrl::set_root_dir(const wxString& dir)
{
    if(dir.IsEmpty()) return;
    wxDir directory(dir);
    if (!directory.IsOpened()) return;

    DeleteAllItems();
    HMB_DNAME = dir;

    wxTreeItemId root = AddRoot(wxFileName(dir).GetFullName());
    populate_tree(dir, root);
    Expand(root);
}

void hmbTreeCtrl::populate_tree(const wxString& path, wxTreeItemId parent)
{
    wxDir directory(path);
    if (!directory.IsOpened())
        return;
    
    wxArrayString dirs;
    wxArrayString files;

    wxString filename;
    bool cont = directory.GetFirst(&filename);
    
    while (cont)
    {
        wxString fullPath = path + wxFileName::GetPathSeparator() + filename;
        if (wxDir::Exists(fullPath)) dirs.Add(filename);
        else files.Add(filename);
        cont = directory.GetNext(&filename);
    }

    dirs.Sort();
    files.Sort();

    for (const auto& dirName : dirs)
    {
        wxString fullPath = path + wxFileName::GetPathSeparator() + dirName;
        wxTreeItemId folder = this->AppendItem(parent, dirName);
        populate_tree(fullPath, folder);
    }

    for (const auto& fileName : files)
    {
        wxString fullPath = path + wxFileName::GetPathSeparator() + fileName;
        wxTreeItemId fileItem = this->AppendItem(parent, fileName);
        this->SetItemData(fileItem, new FileItemData(fullPath));
    }
}

// Регистрация подписчика на событие выбора файла.
void hmbTreeCtrl::bind_panelview_ptr(hmbPanelView* panel_view)
{
    this->panel_view_ptr = panel_view;
}

// Выбор элемента в дереве по полному пути к файлу. Если файл найден, он будет выбран, иначе - ничего не произойдет.
// Если файл найден и выбран, это вызовет событие выбора, которое загрузит файл в панель просмотра.
void hmbTreeCtrl::select_item(const wxString& filePath)
{
    if (filePath.IsEmpty()) return;
    wxTreeItemId item = find_item_by_path(this, this->GetRootItem(), filePath);
    if (item.IsOk()) this->SelectItem(item);
}


wxString hmbTreeCtrl::get_current_dir() const
{
    if (HMB_FNAME.IsEmpty()) return HMB_DNAME;
    return wxFileName(HMB_FNAME).GetPath();
}

void hmbTreeCtrl::on_selection(wxTreeEvent& event)
{
    // Получаем идентификатор выбранного узла из системного события wxTreeCtrl.
    wxTreeItemId itemId = event.GetItem();
    if (!itemId.IsOk())
    {
        event.Skip();
        return;
    }
    
    // Проверяем, есть ли у элемента данные (это файл)
    auto pData = static_cast<FileItemData*>(this->GetItemData(itemId));
    if (!pData) return;
    auto fname = pData->GetFilePath();
    if (!fname.EndsWith(".md"))
    {
        wxMessageBox(fname, "URL", wxOK | wxICON_INFORMATION, this);
    } else {
        HMB_FNAME = pData->GetFilePath();
        if (this->panel_view_ptr) this->panel_view_ptr->load_file(HMB_FNAME);
    }

    event.Skip();
}
