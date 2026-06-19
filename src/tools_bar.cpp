#include "tools_bar.h"
#include "bitmaps/save.xpm"
#include <wx/settings.h>
#include <wx/sizer.h>


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
    toolbar->AddTool(wxID_SAVE, wxEmptyString, wxBitmap(save_xpm), _("Save"));
    toolbar->Realize();

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(topBorder, 0, wxEXPAND);
    sizer->Add(toolbar, 0, wxEXPAND | wxLEFT | wxTOP, 2);
    sizer->Add(bottomBorder, 0, wxEXPAND);
    SetSizerAndFit(sizer);
}
