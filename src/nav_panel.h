#pragma once

#include "wx/panel.h"
#include "wx/sizer.h"

#include "tree_viewer.h"


class NavPanel : public wxPanel
{
public:
    explicit NavPanel(wxWindow* parent, hmbRich* txtRich = nullptr);

    TreeViewer* get_tree_viewer() const;

private:
    TreeViewer* tree_viewer;
};