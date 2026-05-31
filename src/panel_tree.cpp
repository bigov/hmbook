#include "hmb/panel_tree.h"
#include "hmb/tree.h"

hmbPanelTree::hmbPanelTree(wxWindow* parent) :
    wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME)
{
    this->tree_viewer = new hmbTree(this);
    this->tree_viewer->SetMinSize(wxSize(40, -1)); // минимальная ширина панели

    // Чтобы иметь возможность для обеих панелей одинаково настраивать
    // размеры и цвет полей и рамок, TreeViewer вложен в wxBoxSizer.
    // дерево занимает всю полезную площадь контейнера.
    wxBoxSizer* navSizer = new wxBoxSizer(wxVERTICAL);
    navSizer->Add(this->tree_viewer, 1, wxEXPAND);
    this->SetSizer(navSizer);
}

hmbPanelTree::~hmbPanelTree()
{
    this->bind_subscriber(nullptr);  // отключить подписку
    if (this->tree_viewer)
    {
        delete this->tree_viewer;
        this->tree_viewer = nullptr;
    }
}

void hmbPanelTree::bind_subscriber(hmbPanelView* subscriber)
{
    // hmbTree будет вызывать интерфейс подписчика (при выборе файла).
    this->tree_viewer->bind_subscriber(subscriber);
}

void hmbPanelTree::set_root_dir(const wxString& dir)
{
    this->tree_viewer->set_root_dir(dir);
}

// Установка курсора на указанный файл в дереве, если он существует
void hmbPanelTree::set_cursor_to(const wxString& current_file)
{
    if (!current_file.empty() && wxFileExists(current_file))
    {
        this->tree_viewer->select_item(current_file);
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
        this->tree_viewer->set_root_dir(path);
    }
}
