#ifndef HMB_PANEL_TREE_H
#define HMB_PANEL_TREE_H

#include "wx/dirdlg.h"
//#include "wx/filename.h"
//#include "wx/dir.h"
#include "wx/panel.h"
#include "wx/sizer.h"

#include "tree.h"


class hmbPanelTree : public wxPanel
{
public:
    explicit hmbPanelTree(wxWindow* parent);

    hmbTree* get_tree_viewer() const;
    wxString get_current_dir() const;
    void open_dir();
    void load_directory(const wxString& dir) {
        this->tree_viewer->load_directory(dir);
    }

private:
    hmbTree* tree_viewer = nullptr;
};

#endif // HMB_PANEL_TREE_H