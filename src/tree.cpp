#include "wx/dir.h"
#include "wx/filename.h"

#include "hmb/tree.h"

hmbTree::hmbTree(wxWindow* parent)
    : wxTreeCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                 wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT | wxTR_SINGLE | wxBORDER_NONE)
{
    // Привязываем событие выбора элемента в дереве
    Bind(wxEVT_TREE_SEL_CHANGED, &hmbTree::on_selection, this);
}

void hmbTree::bind_file_selected_handler(hmbSubscriber* subscriber)
{
    // Регистрация/замена подписчика на выбор файла.
    // Если subscriber == nullptr, уведомления временно отключаются.
    this->file_selected_subscriber = subscriber;
}

void hmbTree::load_directory(const wxString& dir)
{
    if(dir.IsEmpty()) return;

    DeleteAllItems();
    this->current_dir = dir;

    wxDir directory(dir);
    if (!directory.IsOpened())
        return;
    
    wxTreeItemId root = AddRoot(wxFileName(dir).GetFullName());
    populate_tree(dir, root);
    Expand(root);
}

wxString hmbTree::get_current_dir() const
{
    return this->current_dir;
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
            wxTreeItemId folder = AppendItem(parent, filename);
            populate_tree(fullPath, folder);
        }
        else
        {
            // Это файл - добавляем как лист с полным путем в данных
            wxTreeItemId fileItem = AppendItem(parent, filename);
            SetItemData(fileItem, new FileItemData(fullPath));
        }
        
        cont = directory.GetNext(&filename);
    }
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
    FileItemData* pData = static_cast<FileItemData*>(GetItemData(itemId));
    if (pData)
    {
        // Сохраняем текущий выбранный файл в состоянии дерева.
        this->current_file = pData->GetFilePath();

        // Прямой вызов подписчика без промежуточного wx-слоя событий.
        if (this->file_selected_subscriber)
        {
            this->file_selected_subscriber->load_file(this->current_file);
        }
    }

    event.Skip();
}
