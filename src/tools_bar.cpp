#include "tools_bar.h"
#include "bitmaps/save.xpm"
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/artprov.h>
#include <wx/bmpbndl.h>


hmbToolsBar::hmbToolsBar(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
{
    this->init_toolsbar();
    this->save_btn_enable(false);
}

void hmbToolsBar::init_toolsbar()
{
    SetBackgroundColour(*wxWHITE);

    wxPanel* topBorder = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 1));
    wxPanel* bottomBorder = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 1));
    topBorder->SetBackgroundColour("#AAAAAA");
    bottomBorder->SetBackgroundColour("#AAAAAA");

    toolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxTB_HORIZONTAL | wxTB_FLAT | wxTB_TEXT | wxTB_NODIVIDER);
    toolbar->SetBackgroundColour(*wxWHITE);

    toolbar->AddTool(wxID_OPEN, wxEmptyString, wxArtProvider::GetBitmapBundle(wxART_FOLDER_OPEN, wxART_TOOLBAR),
        _("Open dir [Ctrl+O]"), wxITEM_NORMAL);

    toolbar->AddTool(wxID_SAVE, wxEmptyString, wxArtProvider::GetBitmapBundle(wxART_FILE_SAVE, wxART_TOOLBAR),
        _("Save file [Ctrl+S]"), wxITEM_NORMAL);

    toolbar->AddSeparator();

    toolbar->AddTool(wxID_VIEW_DETAILS, wxEmptyString, wxArtProvider::GetBitmapBundle(wxART_HELP_PAGE, wxART_TOOLBAR),
        _("Debug biffer [Ctrl+D]"), wxITEM_CHECK);

    toolbar->AddSeparator();

    toolbar->AddTool(wxID_EXIT, wxEmptyString, wxArtProvider::GetBitmapBundle(wxART_QUIT, wxART_TOOLBAR),
        _("CLose app [Ctrl+W]"), wxITEM_NORMAL);


    toolbar->Realize();

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(topBorder, 0, wxEXPAND);
    sizer->Add(toolbar, 0, wxEXPAND | wxLEFT, 8);
    sizer->Add(bottomBorder, 0, wxEXPAND);
    SetSizerAndFit(sizer);
}
