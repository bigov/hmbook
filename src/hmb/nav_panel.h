#ifndef HMB_NAV_PANEL_H
#define HMB_NAV_PANEL_H

#include "wx/panel.h"
#include "wx/sizer.h"

#include "tree.h"


class NavPanel : public wxPanel
{
public:
    explicit NavPanel(wxWindow* parent, hmbRich* txtRich = nullptr);

    hmbTree* get_tree_viewer() const;

private:
    hmbTree* tree_viewer;
};

#endif // HMB_NAV_PANEL_H