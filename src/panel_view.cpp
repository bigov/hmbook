#include "panel_view.h"

hmbPanelView::hmbPanelView(wxWindow* parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE)
{
    nbook = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxAUI_NB_BOTTOM | wxAUI_NB_RIGHT | wxBORDER_NONE);
    init_pages();

    SetSizer(new wxBoxSizer(wxVERTICAL));
    GetSizer()->Add(nbook, 1, wxEXPAND | wxALL, 0);
}


// Пример перехвата перехвата события (переключения страниц) в панели
void hmbPanelView::bind_events()
{
    auto action = [this]() {
        switch (nbook->GetSelection())
        {
        case 0:
            //page_rich action    
            break;
        case 1:
            //page_source action    
            break;
        case 2:
            //page_buffer action    
            break;
        default:
            break;
        }
    };

    nbook->Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGED, [action](wxAuiNotebookEvent& event)
    {
        action();
        event.Skip();
    });

}

wxMenu* hmbPanelView::edit_menu()
{
    return this->page_rich->edit_menu();
}

void hmbPanelView::init_pages()
{
    this->page_rich = new hmbRich(nbook);
    this->page_source = new hmbText(nbook);
    this->page_buffer = new hmbText(nbook);

    this->nbook->AddPage(this->page_rich, "Rich", true);
    this->nbook->AddPage(this->page_source, "Source", false);
    this->nbook->AddPage(this->page_buffer, "Buffer", false);

    this->page_source->SetWrapMode(wxSTC_WRAP_WORD);
    this->page_buffer->SetWrapMode(wxSTC_WRAP_NONE);

}

void hmbPanelView::bind_subscriber(hmbStatusBar* status_bar)
{
    this->page_rich->bind_subscriber(status_bar);
}

// Загрузка содержимого из файла.
void hmbPanelView::load_file(const wxString& filePath)
{
    if(filePath.IsEmpty()) return;
    if (!isFileExist(filePath)) return;

    HMB_FNAME = filePath;
    file_read(HMB_FNAME, HMB_SRC_DATA); // загрузить исходный текст в глобальную переменную

    this->page_rich->load_document();
    this->page_source->ClearAll();
    this->page_source->AppendText(wxString::FromUTF8(HMB_SRC_DATA.c_str()));
    this->page_buffer->ChangeValue(this->page_rich->get_buffer());    
}

void hmbPanelView::save_file_as()
{
    this->page_rich->save_file_as();
}
