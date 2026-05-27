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

void hmbPanelTree::bind_subscriber(hmbSubscriber* subscriber)
{
    // hmbTree будет вызывать интерфейс подписчика (при выборе файла).
    this->tree_viewer->bind_subscriber(subscriber);
}

void hmbPanelTree::load_directory(const wxString& dir)
{
    this->tree_viewer->load_directory(dir);
}

void hmbPanelTree::open_dir()
{
    wxString defaultDir = wxGetHomeDir(); // начальная папка
    wxDirDialog dlg(this, "Select directory", defaultDir, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    if (dlg.ShowModal() == wxID_OK) {
        wxString path = dlg.GetPath();
        this->tree_viewer->load_directory(path);
    }
}
