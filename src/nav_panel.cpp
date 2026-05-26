#include "hmb/nav_panel.h"

NavPanel::NavPanel(wxWindow* parent, hmbRich* txtRich) :
    wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME)
{

    // Чтобы иметь возможность для обеих панелей одинаково настраивать
    // размеры и цвет полей и рамок, TreeViewer вложен в wxBoxSizer.
    
    tree_viewer = new hmbTree(this, txtRich);
    // Комфортная минимальная ширина панели навигации для читаемости заголовков.
    tree_viewer->SetMinSize(wxSize(40, -1));

    // Компоновщик панели: дерево занимает всю полезную площадь контейнера.
    wxBoxSizer* navSizer = new wxBoxSizer(wxVERTICAL);
    navSizer->Add(tree_viewer, 1, wxEXPAND);
    this->SetSizer(navSizer);
}

hmbTree* NavPanel::get_tree_viewer() const
{
    return tree_viewer;
}