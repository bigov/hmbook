#include "hmb/panel_view.h"

hmbPanelView::hmbPanelView(wxWindow* parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE)
{
    nbook = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxAUI_NB_BOTTOM | wxAUI_NB_RIGHT | wxBORDER_NONE);
    init_pages();

    auto refreshActivePageContent = [this]() {
        switch (nbook->GetSelection())
        {
        case 1:
            this->page_source->ClearAll();
            this->page_source->AppendText(wxString::FromUTF8(HMB_SRC_DATA.c_str()));
            break;
        case 2:
            this->page_buffer->ChangeValue(this->page_rich->get_buffer());
            break;
        default:
            break;
        }
    };

    nbook->Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGED, [refreshActivePageContent](wxAuiNotebookEvent& event) {
        refreshActivePageContent();
        event.Skip();
    });

    SetSizer(new wxBoxSizer(wxVERTICAL));
    GetSizer()->Add(nbook, 1, wxEXPAND | wxALL, 0);
}

wxMenu* hmbPanelView::get_edit_menu()
{
    return this->page_rich->edit_menu();
}

void hmbPanelView::init_pages()
{
    this->page_rich = new hmbRich(nbook);
    this->page_source = new hmbText(nbook);
    this->page_buffer = new hmbText(nbook);

    this->nbook->AddPage(page_rich, "txt_rich", true);
    this->nbook->AddPage(page_source, "source", false);
    this->page_source->SetWrapMode(wxSTC_WRAP_WORD);
    this->nbook->AddPage(page_buffer, "buffer", false);
}

// Реализация интерфейса метода "подписчик". Вызывается из panel_tree при выборе файла в дереве.
void hmbPanelView::load_file(const wxString& filePath)
{
    this->page_rich->load_file(filePath);

    //this->page_source->ChangeValue(wxString::FromUTF8(HMB_SRC_DATA.c_str()));
    this->page_source->ClearAll();
    this->page_source->AppendText(wxString::FromUTF8(HMB_SRC_DATA.c_str()));
    
    this->nbook->SetSelection(0);
}

void hmbPanelView::save_file_as()
{
    this->page_rich->save_file_as();
}
