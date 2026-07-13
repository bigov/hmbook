#include "panel_tree.h"

hmbPanelTree::hmbPanelTree(wxWindow* parent) :
    wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME)
{
    this->tree_ptr = new hmbTreeCtrl(this);
    this->tree_ptr->SetMinSize(wxSize(40, -1)); // минимальная ширина панели

    // Чтобы иметь возможность для обеих панелей одинаково настраивать
    // размеры и цвет полей и рамок, TreeViewer вложен в wxBoxSizer.
    // дерево занимает всю полезную площадь контейнера.
    wxBoxSizer* navSizer = new wxBoxSizer(wxVERTICAL);
    navSizer->Add(this->tree_ptr, 1, wxEXPAND);
    this->SetSizer(navSizer);
}

hmbPanelTree::~hmbPanelTree()
{
    this->bind_panelview_ptr(nullptr);  // отключить подписку
    if (this->tree_ptr)
    {
        this->tree_ptr = nullptr;
    }
}

void hmbPanelTree::bind_panelview_ptr(hmbPanelView* panel_view)
{
    // hmbTree будет вызывать интерфейс подписчика (при выборе файла).
    this->tree_ptr->bind_panelview_ptr(panel_view);
}

void hmbPanelTree::set_root_dir(const wxString& dir)
{
    this->tree_ptr->set_root_dir(dir);
}


wxString hmbPanelTree::get_current_dir()
{
    return this->tree_ptr->get_current_dir();
}


// Установка курсора на указанный файл в дереве, и его загрузка в панель просмотра.
void hmbPanelTree::set_cursor_to(const wxString& current_file)
{
    if (!current_file.empty() && wxFileExists(current_file))
    {
        this->tree_ptr->select_item(current_file);
    }
}

// Диалог выбора начальной директории для панели навигации
void hmbPanelTree::open_dir()
{
    wxString dir = HMB_DNAME;
    if (dir.empty()) dir = wxGetHomeDir();
    
    wxDirDialog dlg(this, "Select directory", dir, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    if (dlg.ShowModal() == wxID_OK) {
        wxString path = dlg.GetPath();
        this->tree_ptr->set_root_dir(path);
    }
}
