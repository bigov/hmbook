#include "hmb/panel_tree.h"

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

hmbTree* hmbPanelTree::get_tree_viewer() const
{
    return this->tree_viewer;
}

wxString hmbPanelTree::get_current_dir() const
{
    return this->tree_viewer->get_current_dir();
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
