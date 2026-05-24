#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <cstring>

#include "txt_rich.h"

bool isFileExist(const wxString filePath)
{
    if (wxFileExists(filePath)) return true;
    wxLogError(_("Not found file '%s'."), filePath.wc_str());
    return false;
}

void load_file_content(const wxString filePath, std::string& content)
{
    content.clear();
    const wchar_t* f = filePath.wc_str();
    if (std::ifstream reader{f}; reader)
   {
        std::string line;
        while (std::getline(reader, line)) content.append(line + "\n");
    } else {
        wxLogError(_("Cannot read file '%s'."), filePath.wc_str());
    }
}


namespace
{
    std::string to_utf8(const wxString& value)
    {
        const wxScopedCharBuffer utf8 = value.ToUTF8();
        return utf8.data() ? std::string(utf8.data()) : std::string();
    }

    std::string replace_placeholder(std::string tpl, const std::string& placeholder, const std::string& content)
    {
        size_t pos = 0;
        while ((pos = tpl.find(placeholder, pos)) != std::string::npos)
        {
            tpl.replace(pos, placeholder.size(), content);
            pos += content.size();
        }
        return tpl;
    }

} // namespace

// Конструктор класса TxtCtl
TxtRich::TxtRich(wxWindow* parent)
    : wxRichTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
                    wxBORDER_NONE | wxWANTS_CHARS)
{
    this->style_sheet = std::make_unique<wxRichTextStyleSheet>();

    // Base character style
    this->defCharBase = new wxRichTextCharacterStyleDefinition("CharBase");
    auto style_base = &defCharBase->GetStyle();
    style_base->SetFlags(wxTEXT_ATTR_FONT_POINT_SIZE | wxTEXT_ATTR_FONT | wxTEXT_ATTR_TEXT_COLOUR | wxTEXT_ATTR_BACKGROUND_COLOUR);
    //style_base->SetFont(wxFontInfo(14).FaceName("Adwaita Sans Text"));
    style_base->SetFont(this->font_base);
    style_base->SetTextColour(this->color_base_fg);
    style_base->SetBackgroundColour(this->color_base_bg);
    style_base->SetLeftIndent(20);
    style_base->SetRightIndent(10);
    style_base->SetCharacterStyleName("CharBase");
    this->style_sheet->AddCharacterStyle(this->defCharBase);

    // Basic style defines the document-wide baseline appearance.
    // SetDefaultStyle влияет только на последующий ввод, а за стиль всего буфера отвечает SetBasicStyle.
    // Рекомендуемая схема:
    //  - сначала SetFont(font_base),
    //  - затем SetBasicStyle с нужными цветами,
    //  - отдельно формировать default стиль для ввода как объединение char+paragraph атрибутов.
    this->SetBasicStyle(this->defCharBase->GetStyle());  // стиль буфера по-умолчанию
    this->SetDefaultStyle(this->defCharBase->GetStyle());
        
    // Style for code blocks
    this->defCharCoBl = new wxRichTextCharacterStyleDefinition("CharCoBl");
    auto style_code_block = &defCharCoBl->GetStyle();
    style_code_block->SetFlags(wxTEXT_ATTR_FONT | wxTEXT_ATTR_TEXT_COLOUR | wxTEXT_ATTR_BACKGROUND_COLOUR);
    style_code_block->SetFont(this->font_mono);
    style_code_block->SetTextColour(this->color_code_fg);
    this->style_sheet->AddCharacterStyle(this->defCharCoBl);

    // Style for inline code
    this->defCharCoLn = new wxRichTextCharacterStyleDefinition("CharCoLn");
    auto style_code_inline = &defCharCoLn->GetStyle();
    style_code_inline->SetFlags(wxTEXT_ATTR_FONT | wxTEXT_ATTR_TEXT_COLOUR | wxTEXT_ATTR_BACKGROUND_COLOUR);
    style_code_inline->SetFont(this->font_mono);
    style_code_inline->SetTextColour(this->color_code_fg);
    style_code_inline->SetBackgroundColour(this->color_gray_bg);
    this->style_sheet->AddCharacterStyle(this->defCharCoLn);

    // Style for links
    this->defCharLink = new wxRichTextCharacterStyleDefinition("CharLink");
    auto style_link = &defCharLink->GetStyle();
    style_link->SetFlags(wxTEXT_ATTR_FONT | wxTEXT_ATTR_TEXT_COLOUR | wxTEXT_ATTR_BACKGROUND_COLOUR);
    style_link->SetFont(this->font_base);
    style_link->SetTextColour(this->color_urls_fg);
    style_link->SetFontUnderlined(true);
    this->style_sheet->AddCharacterStyle(this->defCharLink);

    // Base paragraph style
    this->defParaBase = new wxRichTextParagraphStyleDefinition("ParaBase");
    auto style_para_base = &defParaBase->GetStyle();
    style_para_base->SetFlags(wxTEXT_ATTR_ALIGNMENT |
         wxTEXT_ATTR_LEFT_INDENT | wxTEXT_ATTR_RIGHT_INDENT |
         wxTEXT_ATTR_PARA_SPACING_BEFORE | wxTEXT_ATTR_PARA_SPACING_AFTER );
    style_para_base->SetAlignment(wxTEXT_ALIGNMENT_LEFT);
    style_para_base->SetLeftIndent(0);
    style_para_base->SetRightIndent(0);
    style_para_base->SetParagraphSpacingBefore(0);
    style_para_base->SetParagraphSpacingAfter(0);
    this->style_sheet->AddParagraphStyle(this->defParaBase);
    this->SetDefaultStyle(this->defParaBase->GetStyle());

    // Style for headers
    this->defParaHead = new wxRichTextParagraphStyleDefinition("ParaHead");
    auto style_heading = &defParaHead->GetStyle();
    style_heading->SetFlags(wxTEXT_ATTR_ALIGNMENT |
         wxTEXT_ATTR_LEFT_INDENT | wxTEXT_ATTR_RIGHT_INDENT |
         wxTEXT_ATTR_PARA_SPACING_BEFORE | wxTEXT_ATTR_PARA_SPACING_AFTER );
    style_heading->SetAlignment(wxTEXT_ALIGNMENT_LEFT);
    style_heading->SetLeftIndent(20);
    style_heading->SetRightIndent(10);
    style_heading->SetParagraphSpacingBefore(20);
    style_heading->SetParagraphSpacingAfter(0);
    style_heading->SetCharacterStyleName("CharBase");
    //this->defParaHead->SetStyle(*style_heading);
    this->style_sheet->AddParagraphStyle(this->defParaHead);

    this->GetBuffer().SetStyleSheet(this->style_sheet.get());
    this->ApplyStyleSheet(this->style_sheet.get());

    new_document();
}


TxtRich::~TxtRich()
{
    // Буфер rich text хранит «сырой» указатель на style sheet и сам его не удаляет.
    // Перед уничтожением style_sheet явно обнуляем ссылку в буфере.
    this->GetBuffer().SetStyleSheet(nullptr);
}

void TxtRich::new_document()
{
    this->Clear();
    this->SetInsertionPoint(0);
    this->row_current = 0;
    this->row_total = 0;
}

void TxtRich::load_xml_handler()
{
    if (!wxRichTextBuffer::FindHandler(wxRICHTEXT_TYPE_XML))
    {
        wxRichTextBuffer::AddHandler(new wxRichTextXMLHandler);
    }
}

// --- Save the buffer content as plain text ---
void TxtRich::save_plain_file(const wxString filePath)
{
    wxRichTextBuffer& buffer = this->GetBuffer();
    const wxString plain_text = buffer.GetText().utf8_str();
    wxFileOutputStream output_stream(filePath);
    output_stream.Write(plain_text.data(), plain_text.length());

    if (output_stream.GetLastError() != wxSTREAM_NO_ERROR)
    {
        wxLogError(_("Error while writing to file '%s'."), filePath.wc_str());
        return;
    }
}

void TxtRich::save_xml_file(const wxString filePath)
{
    wxRichTextBuffer& buffer = this->GetBuffer();
    load_xml_handler();
    if (!buffer.SaveFile(filePath, wxRICHTEXT_TYPE_XML))
    {
        wxLogError(_("Cannot save rich buffer file '%s'."), filePath.wc_str());
    }
    return;
}

wxString TxtRich::export_xml_text()
{
    wxRichTextBuffer& buffer = this->GetBuffer();
    load_xml_handler();

    wxString xml_text;
    wxStringOutputStream xml_stream(&xml_text);
    if (!buffer.SaveFile(xml_stream, wxRICHTEXT_TYPE_XML))
    {
        wxLogWarning(_("Cannot serialize rich buffer to XML string."));
        return wxEmptyString;
    }

    return xml_text;
}

// --- Load the prepared XML data into the control's buffer ---
void TxtRich::push_xml_data(const wxString& content)
{
    wxStringInputStream xml_stream(content);
    wxRichTextBuffer& buffer = this->GetBuffer();
    load_xml_handler();
    if (!buffer.LoadFile(xml_stream, wxRICHTEXT_TYPE_XML))
    {
        wxLogWarning(_("Cannot load XML from string."));
        SetValue(content);
        return;
    }
    this->Refresh();
}

// --- Load files with any formats ---
void TxtRich::load_file(const wxString filePath)
{
    if(filePath.IsEmpty()) return;

    auto fileName = wxFileName(filePath);
    wxString fileExt = fileName.GetExt();
    fileExt.LowerCase();

    if(fileExt == RICH_BUFFER_EXT) {
        this->load_xml_file(filePath);
    } else if(fileExt == MARK_BUFFER_EXT) {
        this->load_md_file(filePath);
    } else {
        this->load_plain_file(filePath);
    }
    this->current_filePath = filePath;
}

// --- Load the Markdown file text ---
void TxtRich::load_md_file(const wxString filePath)
{
    if (!isFileExist(filePath)) return;
    load_file_content(filePath, file_content);

    cmark_node* node = cmark_parse_document(file_content.c_str(), file_content.size(), CMARK_OPT_DEFAULT);
    if (!node) {
        wxLogError(_("Error parsing file '%s'."), filePath.wc_str());
        node = nullptr;
        return;
    }

    this->BeginSuppressUndo();
    display_node(node);
    
    // Дополнить пустые строки до конца документа.
    while (this->row_current < this->row_total) {
        append_line();
    }
    
    // Парсер игнорит в файле завешающий '\n'. Фикс - добавление последней строки.
    if (file_content.size() >= 1 && file_content.substr(file_content.size()-1) == "\n") append_line();
    cmark_node_free(node);
    this->EndSuppressUndo();

}

// --- Load the plain text content from a file ---
void TxtRich::load_plain_file(const wxString filePath)
{
    if (!isFileExist(filePath)) return;
    load_file_content(filePath, file_content);

    new_document();
    this->BeginSuppressUndo();
    this->WriteText(wxString::FromUTF8(file_content.c_str()));
    this->EndSuppressUndo();
    this->SetInsertionPoint(0);
}

// --- Load the XML content from a file ---
void TxtRich::load_xml_file(const wxString filePath)
{
    if (!isFileExist(filePath)) return;
    load_file_content(filePath, file_content);
    push_xml_data(file_content);
}

// Переход на следующую строку.
void TxtRich::append_line() {
    Newline();
    this->row_current++;
}

void TxtRich::row_check(cmark_node* node) {
    if(!node) return;
    int row_begin = cmark_node_get_start_line(node);
    while (row_begin > this->row_current)
    {
        Newline();
        this->row_current++;
    }
}

// Содержимое текстовых узлов, code, html_inline и т.д.
// rtc->SetValue(wxString::FromUTF8(u8"äöü — пример"));
void TxtRich::show_literal(cmark_node* node) {
    const char* lit = cmark_node_get_literal(node);
    if (lit && *lit) this->WriteText(wxString::FromUTF8(lit));
}

void TxtRich::md_none(cmark_node* node) {
    this->WriteText("ERROR: Not found node\n");
}

void TxtRich::md_blockquote(cmark_node* node) {
    this->WriteText("Block quote\n");
}

void TxtRich::md_list(cmark_node* node) {
    this->WriteText("List\n");
}

void TxtRich::md_item(cmark_node* node) {
    this->WriteText("Item\n");
}

void TxtRich::md_code_block(cmark_node* node) {
    wxRichTextAttr attr_bg;
    attr_bg.SetBackgroundColour(this->color_gray_bg);
    wxTextBoxAttr& tba = attr_bg.GetTextBoxAttr();
    tba.GetWidth().SetValue(100, wxTEXT_ATTR_UNITS_PERCENTAGE);
    tba.GetRightMargin().SetValue(10, wxTEXT_ATTR_UNITS_POINTS);
    tba.GetLeftPadding().SetValue(10, wxTEXT_ATTR_UNITS_POINTS);
    tba.GetRightPadding().SetValue(10, wxTEXT_ATTR_UNITS_POINTS);

    wxRichTextBox* box = this->WriteTextBox(attr_bg);
    if (!box) return;

    if (this->defCharCoBl)
    {
        box->SetDefaultStyle(this->defCharCoBl->GetStyle());
    }
    // Многострочный литерал
    const char* lit = cmark_node_get_literal(node);

    if (lit && *lit) {
        std::istringstream ss(lit);
        std::string line;
        while (std::getline(ss, line)) {
            box->AddParagraph(wxString::FromUTF8(line.c_str()));
            this->row_current++;
        }
        box->AddParagraph(wxEmptyString);
        this->row_current++;
    }
}

void TxtRich::md_html_block(cmark_node* node) {
    this->WriteText("HTML block\n");
}
void TxtRich::md_custom_block(cmark_node* node) {
    this->WriteText("Custom block\n");
}

void TxtRich::md_header(cmark_node* node) {
    int font_size = 18 - cmark_node_get_heading_level(node) * 2;
    wxFont f(wxFontInfo(font_size).Weight(wxFONTWEIGHT_BOLD));

    this->BeginParagraphStyle("ParaHead");
    this->BeginFont(f);

    // Текст заголовка — в первой дочерней текстовой ноде
    display_node(cmark_node_first_child(node));
    
    this->EndFont();
    this->WriteText("\n");
    this->row_current++;
    this->EndParagraphStyle();
}

void TxtRich::md_thematic_break(cmark_node* node) {
    this->WriteText("Thematic break\n");
}

void TxtRich::md_text(cmark_node* node) {
    show_literal(node);
}

void TxtRich::md_code_inline(cmark_node* node) {
    if (this->defCharCoLn)
    {
        this->BeginStyle(this->defCharCoLn->GetStyle());
    }
    this->WriteText(" ");
    show_literal(node);
    this->WriteText(" ");
    if (this->defCharCoLn)
    {
        this->EndStyle();
    }
}
void TxtRich::md_html_inline(cmark_node* node) {
    this->WriteText("HTML inline\n");
}
void TxtRich::md_custom_inline(cmark_node* node) {
    this->WriteText("Custom inline\n");
}
void TxtRich::md_emph(cmark_node* node) {
    this->BeginItalic();
    display_node(cmark_node_first_child(node));
    this->EndItalic();
}
void TxtRich::md_strong(cmark_node* node) {
    this->BeginBold();
    display_node(cmark_node_first_child(node));
    this->EndBold();
}
void TxtRich::md_link(cmark_node* node) {
    const char *url = cmark_node_get_url(node);
    //const char *title = cmark_node_get_title(node);
    //const char *text = cmark_node_get_literal(cmark_node_first_child(node));
    this->BeginURL(url, "style_urls");
    display_node(cmark_node_first_child(node));
    //this->WriteText(wxString::FromUTF8(text));
    this->EndURL();
}
void TxtRich::md_image(cmark_node* node) {
    this->WriteText("Image\n");
}
void TxtRich::md_unknown(cmark_node* node) {
    this->WriteText("Unknown\n");
}

void TxtRich::md_paragraph(cmark_node* node) {
    row_check(node);
    this->BeginParagraphStyle("ParaBase");
    node = cmark_node_first_child(node);
    while (node)
    {
      display_node(node);
      node = cmark_node_next(node);
    }
    this->EndParagraphStyle();
}

// ---------------------------------------------
// --- диапазон строк ноды (для отладки) ---
void TxtRich::dbg_node(cmark_node* node, const char* info) {
    int start_line = 0;
    int end_line = 0;
    if (node) 
    { 
        start_line = cmark_node_get_start_line(node);
        end_line = cmark_node_get_end_line(node);
    }
    std::cerr << "[" << start_line << " - " << end_line << "] " << info << "\n";
}

void TxtRich::display_node(cmark_node* node)
{
  if (!node) return;
  cmark_node_type t = cmark_node_get_type(node);

 bool D = true;

  switch (t) {
  case CMARK_NODE_NONE:
    if (D) dbg_node(node, "NONE");
    md_none(node);
    break;
  // -- Block nodes --
  case CMARK_NODE_DOCUMENT:
    if (D) dbg_node(node, "DOCUMENT");
    new_document();
    this->row_current = cmark_node_get_start_line(node);
    this->row_total = cmark_node_get_end_line(node);
    break;
  case CMARK_NODE_HEADING:
    if (D) dbg_node(node, "HEADING");
    md_header(node);
    node = nullptr;
    break;
  case CMARK_NODE_PARAGRAPH:
    if (D) dbg_node(node, "PARAGRAPH");
    md_paragraph(node);
    node = nullptr;
    break;
  case CMARK_NODE_BLOCK_QUOTE:
    if (D) dbg_node(node, "BLOCK_QUOTE");
    md_blockquote(node);
    break;
  case CMARK_NODE_LIST:
    if (D) dbg_node(node, "LIST");
    md_list(node);
    break;
  case CMARK_NODE_ITEM:
    if (D) dbg_node(node, "ITEM");
    md_item(node);
    break;
  case CMARK_NODE_CODE_BLOCK:
    if (D) dbg_node(node, "CODE_BLOCK");
    md_code_block(node);
    break;
  case CMARK_NODE_HTML_BLOCK:
    if (D) dbg_node(node, "HTML_BLOCK");
    md_html_block(node);
    break;
  case CMARK_NODE_CUSTOM_BLOCK:
    if (D) dbg_node(node, "CUSTOM_BLOCK");
    md_custom_block(node);
    break;
  case CMARK_NODE_THEMATIC_BREAK:
    if (D) dbg_node(node, "THEMATIC_BREAK");
    md_thematic_break(node);
    break;
    break;
  // -- Inline nodes --
  case CMARK_NODE_TEXT:
    if (D) dbg_node(node, "TEXT");
    md_text(node);
    break;
  case CMARK_NODE_SOFTBREAK:
    if (D) dbg_node(node, "SOFTBREAK");
    this->WriteText("\n");
    this->row_current++;
    break;
  case CMARK_NODE_LINEBREAK:
    if (D) dbg_node(node, "LINEBREAK");
    this->WriteText("\n");
    this->row_current++;
    break;
  case CMARK_NODE_CODE:
    if (D) dbg_node(node, "CODE");
    md_code_inline(node);
    break;
  case CMARK_NODE_HTML_INLINE:
    if (D) dbg_node(node, "HTML_INLINE");
    md_html_inline(node);
    break;
  case CMARK_NODE_CUSTOM_INLINE:
    if (D) dbg_node(node, "CUSTOM_INLINE");
    md_custom_inline(node);
    break;
  case CMARK_NODE_EMPH:
    if (D) dbg_node(node, "EMPH");
    md_emph(node);
    break;
  case CMARK_NODE_STRONG:
    if (D) dbg_node(node, "STRONG");
    md_strong(node);
    break;
  case CMARK_NODE_LINK:
    if (D) dbg_node(node, "LINK");
    md_link(node);
    break;
  case CMARK_NODE_IMAGE:
    if (D) dbg_node(node, "IMAGE");
    md_image(node);
    break;
  default:
    if (D) dbg_node(node, "UNKNOWN");
    md_unknown(node);
    break;
  }

    // Рекурсивный обход
    cmark_node* child = cmark_node_first_child(node);
    while (child) {
        display_node(child);
        child = cmark_node_next(child);
    }
}

wxMenu* TxtRich::edit_menu()
{
    wxWindow* topLevel = wxGetTopLevelParent(this);
    wxMenu* editMenu = new wxMenu;
    
    editMenu->Append(RICHTEXT_LEFT_ALIGN, _("Left Align"));
    if (topLevel)
        topLevel->Bind(wxEVT_MENU, [this](wxCommandEvent& event){ on_left_align(event); }, RICHTEXT_LEFT_ALIGN);
    
    editMenu->Append(RICHTEXT_RIGHT_ALIGN, _("Right Align"));
    if (topLevel)
        topLevel->Bind(wxEVT_MENU, [this](wxCommandEvent& event){ on_right_align(event); }, RICHTEXT_RIGHT_ALIGN);
    
    editMenu->Append(RICHTEXT_CENTRE, _("Centre"));
    if (topLevel)
        topLevel->Bind(wxEVT_MENU, [this](wxCommandEvent& event){ OnCentre(event); }, RICHTEXT_CENTRE);
    
    editMenu->Append(RICHTEXT_JUSTIFY, _("Justify"));
    if (topLevel)
        topLevel->Bind(wxEVT_MENU, [this](wxCommandEvent& event){ OnJustify(event); }, RICHTEXT_JUSTIFY);
    
    editMenu->AppendSeparator();
    
    editMenu->Append(RICHTEXT_CHANGE_FONT, _("Change Font"));
    if (topLevel)
        topLevel->Bind(wxEVT_MENU, [this](wxCommandEvent& event){ on_change_font(event); }, RICHTEXT_CHANGE_FONT);
    
    editMenu->Append(RICHTEXT_CHANGE_TEXT_COLOUR, _("Change Text Colour"));
    if (topLevel)
        topLevel->Bind(wxEVT_MENU, [this](wxCommandEvent& event){ OnChangeTextColour(event); }, RICHTEXT_CHANGE_TEXT_COLOUR);
    
    editMenu->Append(RICHTEXT_CHANGE_BACKGROUND_COLOUR, _("Change Background Colour"));
    if (topLevel)
        topLevel->Bind(wxEVT_MENU, [this](wxCommandEvent& event){ OnChangeBackgroundColour(event); }, RICHTEXT_CHANGE_BACKGROUND_COLOUR);
    
    editMenu->AppendSeparator();
    
    editMenu->Append(RICHTEXT_LEFT_INDENT, _("Left Indent"));
    if (topLevel)
        topLevel->Bind(wxEVT_MENU, [this](wxCommandEvent& event){ OnLeftIndent(event); }, RICHTEXT_LEFT_INDENT);

    editMenu->Append(RICHTEXT_RIGHT_INDENT, _("Right Indent"));
    if (topLevel)
        topLevel->Bind(wxEVT_MENU, [this](wxCommandEvent& event){ OnRightIndent(event); }, RICHTEXT_RIGHT_INDENT);

    editMenu->Append(RICHTEXT_TAB_STOPS, _("Tab Stops"));
    if (topLevel)
        topLevel->Bind(wxEVT_MENU, [this](wxCommandEvent& event){ OnTabStops(event); }, RICHTEXT_TAB_STOPS);

    return editMenu;
}


void TxtRich::on_change_font(wxCommandEvent& WXUNUSED(event))
{
    wxFontData data;
    data.EnableEffects(true);
    wxFontDialog dialog(this, data);

    if (dialog.ShowModal() == wxID_OK)
    {
        wxFontData retData = dialog.GetFontData();
        wxFont font = retData.GetChosenFont();
        wxColour colour = retData.GetColour();

        wxTextAttr attr;
        attr.SetFont(font);
        if (colour.IsOk())
            attr.SetTextColour(colour);

        long start, end;
        this->GetSelection(& start, & end);
        this->SetStyle(start, end, attr);
    }
}

void TxtRich::on_left_align(wxCommandEvent& WXUNUSED(event))
{
    wxTextAttr attr;
    attr.SetAlignment(wxTEXT_ALIGNMENT_LEFT);

    long start, end;
    this->GetSelection(& start, & end);
    this->SetStyle(start, end, attr);
}

void TxtRich::on_right_align(wxCommandEvent& WXUNUSED(event))
{
    wxTextAttr attr;
    attr.SetAlignment(wxTEXT_ALIGNMENT_RIGHT);

    long start, end;
    this->GetSelection(& start, & end);
    this->SetStyle(start, end, attr);
}

void TxtRich::OnJustify(wxCommandEvent& WXUNUSED(event))
{
    wxTextAttr attr;
    attr.SetAlignment(wxTEXT_ALIGNMENT_JUSTIFIED);

    long start, end;
    this->GetSelection(& start, & end);
    this->SetStyle(start, end, attr);
}

void TxtRich::OnCentre(wxCommandEvent& WXUNUSED(event))
{
    wxTextAttr attr;
    attr.SetAlignment(wxTEXT_ALIGNMENT_CENTRE);

    long start, end;
    this->GetSelection(& start, & end);
    this->SetStyle(start, end, attr);
}


void TxtRich::OnChangeTextColour(wxCommandEvent& WXUNUSED(event))
{
    wxColourData data;
    data.SetColour(* wxBLACK);
    data.SetChooseFull(true);
    for (int i = 0; i < 16; i++)
    {
        wxColour colour((unsigned char)(i*16), (unsigned char)(i*16), (unsigned char)(i*16));
        data.SetCustomColour(i, colour);
    }

    wxColourDialog dialog(this, &data);
    dialog.SetTitle("Choose the text colour");
    if (dialog.ShowModal() == wxID_OK)
    {
        wxColourData retData = dialog.GetColourData();
        wxColour col = retData.GetColour();

        wxTextAttr attr;
        attr.SetTextColour(col);

        long start, end;
        this->GetSelection(& start, & end);
        this->SetStyle(start, end, attr);
    }
}

void TxtRich::OnChangeBackgroundColour(wxCommandEvent& WXUNUSED(event))
{
    wxColourData data;
    data.SetColour(* wxWHITE);
    data.SetChooseFull(true);
    for (int i = 0; i < 16; i++)
    {
        wxColour colour((unsigned char)(i*16), (unsigned char)(i*16), (unsigned char)(i*16));
        data.SetCustomColour(i, colour);
    }

    wxColourDialog dialog(this, &data);
    dialog.SetTitle("Choose the text background colour");
    if (dialog.ShowModal() == wxID_OK)
    {
        wxColourData retData = dialog.GetColourData();
        wxColour col = retData.GetColour();

        wxTextAttr attr;
        attr.SetBackgroundColour(col);

        long start, end;
        this->GetSelection(& start, & end);
        this->SetStyle(start, end, attr);
    }
}

void TxtRich::OnLeftIndent(wxCommandEvent& WXUNUSED(event))
{
    wxString indentStr = wxGetTextFromUser
                         (
                            _("Please enter the left indent in tenths of a millimetre."),
                            _("Left Indent"),
                            wxEmptyString,
                            this
                         );
    if (!indentStr.IsEmpty())
    {
        int indent = wxAtoi(indentStr);

        wxTextAttr attr;
        attr.SetLeftIndent(indent);

        long start, end;
        this->GetSelection(& start, & end);
        this->SetStyle(start, end, attr);
    }
}

void TxtRich::OnRightIndent(wxCommandEvent& WXUNUSED(event))
{
    wxString indentStr = wxGetTextFromUser
                         (
                            _("Please enter the right indent in tenths of a millimetre."),
                            _("Right Indent"),
                            wxEmptyString,
                            this
                         );
    if (!indentStr.IsEmpty())
    {
        int indent = wxAtoi(indentStr);

        wxTextAttr attr;
        attr.SetRightIndent(indent);

        long start, end;
        this->GetSelection(& start, & end);
        this->SetStyle(start, end, attr);
    }
}

void TxtRich::OnTabStops(wxCommandEvent& WXUNUSED(event))
{
    wxString tabsStr = wxGetTextFromUser
    (
        _("Please enter the tab stop positions in tenths of a millimetre, separated by spaces.\nLeave empty to reset tab stops."),
        _("Tab Stops"), wxEmptyString, this
    );

    wxArrayInt tabs;

    wxStringTokenizer tokens(tabsStr, " ");
    while (tokens.HasMoreTokens())
    {
        wxString token = tokens.GetNextToken();
        tabs.Add(wxAtoi(token));
    }

    wxTextAttr attr;
    attr.SetTabs(tabs);

    long start, end;
    this->GetSelection(& start, & end);
    this->SetStyle(start, end, attr);
}
