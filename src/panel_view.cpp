#include "panel_view.h"

hmbPanelView::hmbPanelView(wxWindow* parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE)
{
    this->nbook = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxAUI_NB_BOTTOM | wxAUI_NB_RIGHT | wxBORDER_NONE);
    init_pages();
    this->nbook->Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGED, &hmbPanelView::page_changed, this);

    SetSizer(new wxBoxSizer(wxVERTICAL));
    GetSizer()->Add(nbook, 1, wxEXPAND | wxALL, 0);
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

// Обработка события переключения страниц в панели вида
void hmbPanelView::page_changed(wxAuiNotebookEvent& event)
{
    switch (this->nbook->GetSelection())
    {
    case 0:
        // При активации вкладки Rich передаем фокус в STC.
        this->page_rich->load_src_data(); // обновить рендер при переключении на вкладку
        //this->page_rich->SetFocus();
        break;
    case 1:
        // При активации вкладки Source передаем фокус в STC.
        //this->page_source->SetFocus();
        break;
    case 2:
        // При активации вкладки Buffer передаем фокус в STC.
        this->page_buffer->ChangeValue(this->page_rich->get_buffer());    
        //this->page_buffer->SetFocus();
        break;
    default:
        break;
    }
    event.Skip();
}


wxMenu* hmbPanelView::edit_menu()
{
    return this->page_rich->edit_menu();
}


void hmbPanelView::bind_subscriber(hmbStatusBar* status_bar)
{
    this->page_rich->bind_subscriber(status_bar);
}


void hmbPanelView::bind_toolsbar(hmbToolsBar* tools_bar)
{
    this->page_source->toolsbar = tools_bar;
}

// Загрузка содержимого из файла.
void hmbPanelView::load_file(const wxString& filePath)
{
    if(filePath.IsEmpty()) return;
    if (!isFileExist(filePath)) return;

    HMB_FNAME = filePath;
    file_read(HMB_FNAME, HMB_SRC_DATA); // загрузить исходный текст в глобальную переменную

    this->page_rich->load_src_data();
    this->page_source->load_src_data();
}

void hmbPanelView::save_file()
{
    file_write(HMB_SRC_DATA, HMB_FNAME);
}
