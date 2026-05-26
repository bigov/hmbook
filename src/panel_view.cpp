#include "hmb/panel_view.h"
#include "hmb/panel_tree.h"


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
            this->page_source->ChangeValue(wxString::FromUTF8(this->page_rich->file_content.c_str()));
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

// Регистрация подписки на выбор файла в дереве hmbPanelTree.
// Далее panel_tree будет вызывать load_file(...) у этого экземпляра.
void hmbPanelView::bind_tree_selection(hmbPanelTree* panelTree)
{
    if (!panelTree) return;
    panelTree->bind_file_selected_handler(this);
}

void hmbPanelView::load_file(const wxString& filePath)
{
    // Реакция подписчика на событие выбора файла:
    // делегируем загрузку выбранного файла в текущую панель просмотра.
    this->page_rich->load_file(filePath);
    this->nbook->SetSelection(0);
}

void hmbPanelView::init_pages()
{
    this->page_rich = new hmbRich(nbook);

    this->page_source = new wxTextCtrl(
        nbook,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxTE_MULTILINE | wxBORDER_NONE | wxTE_NO_VSCROLL
    );

    this->page_buffer = new wxTextCtrl(
        nbook,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxTE_MULTILINE | wxBORDER_NONE | wxTE_NO_VSCROLL
    );

    this->nbook->AddPage(page_rich, "txt_rich", true);
    this->nbook->AddPage(page_source, "source", false);
    this->nbook->AddPage(page_buffer, "buffer", false);

}

void hmbPanelView::save_file_as()
{
    this->page_rich->save_file_as();
}

wxString hmbPanelView::get_current_file() const
{
    return this->page_rich->current_file;
}
