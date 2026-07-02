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
    this->page_text = new hmbText(nbook);

    this->nbook->AddPage(this->page_rich, "Rich", true);
    this->nbook->AddPage(this->page_text, "Text", false);

    this->page_text->SetWrapMode(wxSTC_WRAP_WORD);
}


// Обработка события переключения страниц в панели вида
void hmbPanelView::page_changed(wxAuiNotebookEvent& event)
{
    switch (this->nbook->GetSelection())
    {
    case 0: // страница рендера
        this->page_text->on_edit = false;
        this->page_text->toolsbar->text_mode_enable(false);
        this->page_text->toolsbar->wrap_btn_enable(false);
        this->page_rich->load_src_data(); // обновить рендер при переключении на вкладку
        break;
    case 1: // страница исходного кода
        this->page_text->on_edit = true;
        this->page_text->toolsbar->text_mode_enable(true);
        this->page_text->toolsbar->wrap_btn_enable(true);
        break;
    default:
        break;
    }
    event.Skip();
}


// Переключение источника текста - исходный файл или XML буфер
void hmbPanelView::mode_switch(wxCommandEvent& event)
{
    auto s = std::string("");
    this->page_text->on_edit = false;

    if (event.IsChecked())
    {
        this->page_text->set_lexer(wxSTC_LEX_XML);
        this->page_rich->read_buffer_xml(s);
        this->page_text->load_data(s);
    } else {   
        this->page_text->set_lexer(wxSTC_LEX_MARKDOWN);
        this->page_text->load_data(HMB_SRC_DATA);
    }
}

// Переключить режим переноса слов на вкладке "Text".
void hmbPanelView::toggle_wrap(wxCommandEvent& event)
{
    this->page_text->SetWrapMode(event.IsChecked() ? wxSTC_WRAP_WORD : wxSTC_WRAP_NONE);
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
    this->page_text->toolsbar = tools_bar;
}

// Загрузка содержимого из файла.
void hmbPanelView::load_file(const wxString& filePath)
{
    if(filePath.IsEmpty()) return;
    if (!isFileExist(filePath)) return;

    HMB_FNAME = filePath;
    file_read(HMB_FNAME, HMB_SRC_DATA); // загрузить исходный текст в глобальную переменную

    this->page_rich->load_src_data();
    this->page_text->load_data(HMB_SRC_DATA);
}

void hmbPanelView::save_file()
{
    file_write(HMB_SRC_DATA, HMB_FNAME);
}

