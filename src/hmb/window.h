#ifndef HMB_WINDOW_H
#define HMB_WINDOW_H

#include "wx/frame.h"
#include "wx/string.h"
#include "wx/splitter.h"

#include "rich.h"

class hmbTree;

class hmbWindow: public wxFrame
{
public:
    hmbWindow(const wxString& title);

    void OnClose(wxCommandEvent& event);
    void OnWindowClose(wxCloseEvent& event);
    void OpenDir(wxCommandEvent& event);
    void FileSaveAs(wxCommandEvent& event);

private:
    hmbRich* txt_rich = nullptr;
    hmbTree* tree_viewer = nullptr;
    wxSplitterWindow* splitter;

    void load_params();
    void save_params();
    void SetAppIcon(const wxString& iconPath);
};

#endif // HMB_WINDOW_H