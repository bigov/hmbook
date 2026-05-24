#include "main_panel.h"


MainPanel::MainPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE)
{
    notebook = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxAUI_NB_BOTTOM | wxAUI_NB_RIGHT | wxBORDER_NONE);
    init_pages();

    auto refreshActivePageContent = [this]() {
        switch (notebook->GetSelection())
        {
        case 1:
            page_source->ChangeValue(wxString::FromUTF8(page_rich->file_content.c_str()));
            break;
        case 2:
            page_buffer->ChangeValue(page_rich->get_buffer());
            break;
        default:
            break;
        }
    };

    notebook->Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGED, [refreshActivePageContent](wxAuiNotebookEvent& event) {
        refreshActivePageContent();
        event.Skip();
    });

    SetSizer(new wxBoxSizer(wxVERTICAL));
    GetSizer()->Add(notebook, 1, wxEXPAND | wxALL, 0);
}


void MainPanel::init_pages()
{
    page_rich = new TxtRich(notebook);

    page_source = new wxTextCtrl(
        notebook,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxTE_MULTILINE | wxBORDER_NONE | wxTE_NO_VSCROLL
    );

    page_buffer = new wxTextCtrl(
        notebook,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxTE_MULTILINE | wxBORDER_NONE | wxTE_NO_VSCROLL
    );

    notebook->AddPage(page_rich, "txt_rich", true);
    notebook->AddPage(page_source, "source", false);
    notebook->AddPage(page_buffer, "buffer", false);

}


TxtRich* MainPanel::get_txt_rich() const
{
    return page_rich;
}
