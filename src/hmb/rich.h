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

#include <memory>

#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <cstring>

#include "wx/menu.h"
#include "wx/richtext/richtextctrl.h"
#include "wx/richtext/richtextxml.h"
#include "wx/wfstream.h"
#include "wx/sstream.h"
#include "wx/colordlg.h"
#include "wx/fontdlg.h"
#include "wx/textdlg.h"
#include "wx/tokenzr.h"
#include "wx/log.h"

#include "tools.h"

extern "C" {
#include "cmark.h"
}

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

static const wxString RICH_BUFFER_EXT = "wxrt";
static const wxString TEXT_BUFFER_EXT = "txt";
static const wxString MARK_BUFFER_EXT = "md";

bool isFileExist(const wxString filePath);
void load_file_content(const wxString filePath, std::string& content);

class hmbRich: public wxRichTextCtrl
{
public:
    hmbRich(wxWindow* parent);
    ~hmbRich() override;
    wxString current_filePath;
    std::string file_content;
    wxMenu* edit_menu();
    void load_file(const wxString filePath);
    void save_plain_file(const wxString filePath);
    void save_xml_file(const wxString filePath);
    wxString get_buffer();

private:
    
    wxColor color_base_fg = "#444444";
    wxColor color_urls_fg = "#25A4D1"; 
    wxColor color_code_fg = "#0954b8";
    wxColor color_base_bg = "#ffffff";
    wxColor color_gray_bg = "#f0f0f0";
    
    wxFont font_base = wxFontInfo(11).FaceName("Adwaita Sans Text");
    wxFont font_mono = wxFontInfo(11).FaceName("Adwaita Mono");

    wxRichTextCharacterStyleDefinition* defCharBase = nullptr; // базовый стиль
    wxRichTextCharacterStyleDefinition* defCharLink = nullptr; // ссылки
    wxRichTextCharacterStyleDefinition* defCharCoBl = nullptr; // code block
    wxRichTextCharacterStyleDefinition* defCharCoLn = nullptr; // code inline

    wxRichTextParagraphStyleDefinition* defParaBase = nullptr; // базовый стиль для абзаца
    wxRichTextParagraphStyleDefinition* defParaHead = nullptr; // стиль для заголовков

    // Таблица стилей для всего документа
    std::unique_ptr<wxRichTextStyleSheet> style_sheet = nullptr;

    int row_current = 0;
    int row_total = 0;

    void dbg_node(cmark_node* node, const char* info);
    
    void new_document();
    void push_xml_data(const wxString& xml_data);

    void display_node(cmark_node* node);
    void append_line();
    void row_check(cmark_node* node);
    void md_paragraph(cmark_node* node);
    void show_literal(cmark_node* node);
    void md_none(cmark_node* node);
    void md_blockquote(cmark_node* node);
    void md_list(cmark_node* node);
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
    void load_xml_handler();
    void load_xml_file(const wxString filePath);
    void load_md_file(const wxString filePath);
    void load_plain_file(const wxString filePath);
};

#endif // HMB_RICH_H
