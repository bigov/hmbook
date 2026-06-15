#ifndef HMB_WINDOW_H
#define HMB_WINDOW_H

#include <wx/frame.h>
#include <wx/string.h>
#include <wx/splitter.h>
#include <wx/sstream.h>
#include <wx/filedlg.h>
#include <wx/dirdlg.h>
#include <wx/filename.h>
#include <wx/wfstream.h>
#include <wx/log.h>
#include <wx/filefn.h>
#include <wx/icon.h>
#include <wx/image.h>
#include <wx/menu.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stdpaths.h>
#include <wx/config.h>
#include <wx/dir.h>

#include "panel_tree.h"
#include "panel_view.h"
#include "rich.h"
#include "tools_bar.h"
#include "status_bar.h"

class hmbTree;

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

    void load_params();
    void save_params();
    void SetAppIcon(const wxString& iconPath);
};

#endif // HMB_WINDOW_H