#ifndef HMB_WINDOW_H
#define HMB_WINDOW_H

#include <thread>

#include "panel_tree.h"
#include "panel_view.h"
#include "rich.h"
#include "tools_bar.h"
#include "status_bar.h"

class hmbTreeCtrl;

class hmbWindow: public wxFrame
{
public:
    hmbWindow(const wxString& title);

    void OnClose(wxCommandEvent& event);
    void OnWindowClose(wxCloseEvent& event);

private:
    wxMenuBar* menuBar = nullptr;
    hmbToolsBar* toolsBar = nullptr;
    hmbStatusBar* statusBar = nullptr;

    wxSplitterWindow* splitter = nullptr;
    hmbPanelView* panel_view = nullptr;
    hmbPanelTree* panel_tree = nullptr;

    void git_pull();
    void git_push();
    void load_params();
    void save_params();
    void save_file_data();
    void SetAppIcon(const wxString& iconPath);
};

#endif // HMB_WINDOW_H