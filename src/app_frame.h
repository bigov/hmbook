#pragma once
#include "wx/frame.h"
#include "wx/string.h"
#include "wx/splitter.h"

#include "rich.h"

class TreeViewer;

class AppFrame: public wxFrame
{
public:
    AppFrame(const wxString& title);

    void OnClose(wxCommandEvent& event);
    void OnWindowClose(wxCloseEvent& event);
    void OpenDir(wxCommandEvent& event);
    void FileSaveAs(wxCommandEvent& event);

private:
    hmbRich* txt_rich = nullptr;
    TreeViewer* tree_viewer = nullptr;
    wxSplitterWindow* splitter;

    void load_params();
    void save_params();
    void SetAppIcon(const wxString& iconPath);
};