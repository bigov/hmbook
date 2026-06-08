#ifndef HMB_RICH_H
#define HMB_RICH_H

/**
Related Classes (https://docs.wxwidgets.org/stable/overview_richtextctrl.html):

    Major classes: wxRichTextCtrl, wxRichTextBuffer, wxRichTextEvent
    Helper classes: wxTextAttr, wxRichTextRange
    File handler classes: wxRichTextFileHandler, wxRichTextHTMLHandler, wxRichTextXMLHandler
    Style classes: wxRichTextCharacterStyleDefinition, wxRichTextParagraphStyleDefinition, wxRichTextListStyleDefinition, wxRichTextStyleSheet
    Additional controls: wxRichTextStyleComboCtrl, wxRichTextStyleListBox, wxRichTextStyleListCtrl
    Printing classes: wxRichTextPrinting, wxRichTextPrintout, wxRichTextHeaderFooterData
    Dialog classes: wxRichTextStyleOrganiserDialog, wxRichTextFormattingDialog, wxSymbolPickerDialog
*/

#include "status_bar.h"
#include "tools.h"

class wxMenu;
class wxCommandEvent;

enum
{
    RICHTEXT_LEFT_ALIGN = 1100,
    RICHTEXT_RIGHT_ALIGN,
    RICHTEXT_CENTRE,
    RICHTEXT_JUSTIFY,
    RICHTEXT_CHANGE_FONT,
    RICHTEXT_CHANGE_TEXT_COLOUR,
    RICHTEXT_CHANGE_BACKGROUND_COLOUR,
    RICHTEXT_LEFT_INDENT,
    RICHTEXT_RIGHT_INDENT,
    RICHTEXT_TAB_STOPS
};


class hmbRich: public wxRichTextCtrl
{
public:
    explicit hmbRich(wxWindow* parent);
    ~hmbRich() override;

    wxMenu* edit_menu();
    void load_document();
    void save_file_as();
    void save_plain_file(const wxString filePath);
    wxString get_buffer();
    void bind_subscriber(hmbStatusBar* status_bar);

private:
    wxColor color_urls_fg = "#25A4D1"; 
    wxColor color_code_fg = "#0954b8";
    wxColor color_gray_bg = "#f0f0f0";
    
    wxRichTextCharacterStyleDefinition* defCharBase = nullptr; // базовый стиль
    wxRichTextCharacterStyleDefinition* defCharLink = nullptr; // ссылки
    wxRichTextCharacterStyleDefinition* defCharCoBl = nullptr; // code block
    wxRichTextCharacterStyleDefinition* defCharCoLn = nullptr; // code inline

    wxRichTextParagraphStyleDefinition* defParaBase = nullptr; // базовый стиль для абзаца
    wxRichTextParagraphStyleDefinition* defParaHead = nullptr; // стиль для заголовков

    // Таблица стилей для всего документа
    std::unique_ptr<wxRichTextStyleSheet> style_sheet = nullptr;

    hmbStatusBar* subscriber = nullptr;
    int row_current = 0;
    int row_total = 0;
    bool is_paragraph_open = false;

    void bind_mouse_events();
    void new_document();
    void load_as_plain_text();
    void node_iterator(cmark_node* node);
    void node_dispatcher(cmark_node* node);
    void show_url(const wxString& url);

    void new_line();
    void line_break();
    void row_check(cmark_node* node);
    void md_paragraph(cmark_node* node);
    void md_none(cmark_node* node);
    void md_blockquote(cmark_node* node);
    void md_list(cmark_node* node);
    void md_num_list(cmark_node* node);
    void md_bul_list(cmark_node* node);
    void md_item(cmark_node* node);
    void md_code_block(cmark_node* node);
    void md_html_block(cmark_node* node);
    void md_custom_block(cmark_node* node);
    void md_header(cmark_node* node);
    void md_thematic_break(cmark_node* node);
    void md_text(cmark_node* node);
    void md_code_inline(cmark_node* node);
    void md_html_inline(cmark_node* node);
    void md_custom_inline(cmark_node* node);
    void md_emph(cmark_node* node);
    void md_strong(cmark_node* node);
    void md_link(cmark_node* node);
    void md_image(cmark_node* node);
    void md_unknown(cmark_node* node);

    void on_change_font(wxCommandEvent& WXUNUSED(event));
    void on_left_align(wxCommandEvent& event);
    void on_right_align(wxCommandEvent& event);
    void OnJustify(wxCommandEvent& event);
    void OnCentre(wxCommandEvent& event);
    void OnChangeTextColour(wxCommandEvent& event);
    void OnChangeBackgroundColour(wxCommandEvent& event);
    void OnLeftIndent(wxCommandEvent& event);
    void OnRightIndent(wxCommandEvent& event);
    void OnTabStops(wxCommandEvent& event);
};

#endif // HMB_RICH_H
