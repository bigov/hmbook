#include <wx/dir.h>
#include <wx/filename.h>

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

hmbTree::hmbTree(wxWindow* parent)
    : wxTreeCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                 wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT | wxTR_SINGLE | wxBORDER_NONE)
{
    // Привязываем событие выбора элемента в дереве
    Bind(wxEVT_TREE_SEL_CHANGED, &hmbTree::on_selection, this);
}

void hmbTree::set_root_dir(const wxString& dir)
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

void hmbTree::populate_tree(const wxString& path, wxTreeItemId parent)
{
    wxDir directory(path);
    if (!directory.IsOpened())
        return;
    
    wxString filename;
    bool cont = directory.GetFirst(&filename);
    
    while (cont)
    {
        wxString fullPath = path + wxFileName::GetPathSeparator() + filename;
        
        if (wxDir::Exists(fullPath))
        {
            // Это директория - добавляем как ветку и рекурсивно заполняем
            wxTreeItemId folder = this->AppendItem(parent, filename);
            populate_tree(fullPath, folder);
        }
        else
        {
            // Это файл - добавляем как лист с полным путем в данных
            wxTreeItemId fileItem = this->AppendItem(parent, filename);
            this->SetItemData(fileItem, new FileItemData(fullPath));
        }
        
        cont = directory.GetNext(&filename);
    }
}

// Регистрация подписчика на событие выбора файла.
void hmbTree::bind_subscriber(hmbPanelView* subscriber)
{
    this->subscriber = subscriber;
}

// Выбор элемента в дереве по полному пути к файлу. Если файл найден, он будет выбран, иначе - ничего не произойдет.
// Если файл найден и выбран, это вызовет событие выбора, которое загрузит файл в панель просмотра.
void hmbTree::select_item(const wxString& filePath)
{
    if (filePath.IsEmpty()) return;
    wxTreeItemId item = find_item_by_path(this, this->GetRootItem(), filePath);
    if (item.IsOk()) this->SelectItem(item);
}


void hmbTree::on_selection(wxTreeEvent& event)
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
    if (pData)
    {
        // Сохраняем текущий выбранный файл в состоянии дерева.
        HMB_FNAME = pData->GetFilePath();

        // Вызов интерфейса подписчика для загрузки выбранного файла
        if (this->subscriber) this->subscriber->load_file(HMB_FNAME);
    }

    event.Skip();
}
