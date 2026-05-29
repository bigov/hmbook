#include "wx/filedlg.h"

#include "hmb/rich.h"
#include "hmb/tree.h"
#include "hmb/tools.h"

// Конструктор класса
hmbRich::hmbRich(wxWindow* parent)
    : wxRichTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
                    wxBORDER_NONE | wxWANTS_CHARS | wxVSCROLL| wxALWAYS_SHOW_SB)
{
    this->style_sheet = std::make_unique<wxRichTextStyleSheet>();

    // Base character style
    this->defCharBase = new wxRichTextCharacterStyleDefinition("CharBase");
    auto style_base = &defCharBase->GetStyle();
    style_base->SetFlags(wxTEXT_ATTR_FONT_POINT_SIZE | wxTEXT_ATTR_FONT | wxTEXT_ATTR_TEXT_COLOUR | wxTEXT_ATTR_BACKGROUND_COLOUR);
    //style_base->SetFont(wxFontInfo(14).FaceName("Adwaita Sans Text"));
    style_base->SetFont(HMB_FONT_BASE);
    style_base->SetTextColour(HMB_COLOR_BASE_FG);
    style_base->SetBackgroundColour(HMB_COLOR_BASE_BG);
    style_base->SetLeftIndent(10);
    style_base->SetRightIndent(8);
    style_base->SetCharacterStyleName("CharBase");
    this->style_sheet->AddCharacterStyle(this->defCharBase);

    // Basic style defines the document-wide baseline appearance.
    // SetDefaultStyle влияет только на последующий ввод, а за стиль всего буфера отвечает SetBasicStyle.
    // Рекомендуемая схема:
    //  - сначала SetFont(HMB_FONT_BASE),
    //  - затем SetBasicStyle с нужными цветами,
    //  - отдельно формировать default стиль для ввода как объединение char+paragraph атрибутов.
    this->SetBasicStyle(this->defCharBase->GetStyle());  // стиль буфера по-умолчанию
    this->SetDefaultStyle(this->defCharBase->GetStyle());
        
    // Style for code blocks
    this->defCharCoBl = new wxRichTextCharacterStyleDefinition("CharCoBl");
    auto style_code_block = &defCharCoBl->GetStyle();
    style_code_block->SetFlags(wxTEXT_ATTR_FONT | wxTEXT_ATTR_TEXT_COLOUR | wxTEXT_ATTR_BACKGROUND_COLOUR);
    style_code_block->SetFont(HMB_FONT_MONO);
    style_code_block->SetTextColour(this->color_code_fg);
    this->style_sheet->AddCharacterStyle(this->defCharCoBl);

    // Style for inline code
    this->defCharCoLn = new wxRichTextCharacterStyleDefinition("CharCoLn");
    auto style_code_inline = &defCharCoLn->GetStyle();
    style_code_inline->SetFlags(wxTEXT_ATTR_FONT | wxTEXT_ATTR_TEXT_COLOUR | wxTEXT_ATTR_BACKGROUND_COLOUR);
    style_code_inline->SetFont(HMB_FONT_MONO);
    style_code_inline->SetTextColour(this->color_code_fg);
    style_code_inline->SetBackgroundColour(this->color_gray_bg);
    this->style_sheet->AddCharacterStyle(this->defCharCoLn);

    // Style for links
    this->defCharLink = new wxRichTextCharacterStyleDefinition("CharLink");
    auto style_link = &defCharLink->GetStyle();
    style_link->SetFlags(wxTEXT_ATTR_FONT | wxTEXT_ATTR_TEXT_COLOUR | wxTEXT_ATTR_BACKGROUND_COLOUR);
    style_link->SetFont(HMB_FONT_BASE);
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
    style_para_base->SetLeftIndent(10);
    style_para_base->SetRightIndent(8);
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
    style_heading->SetLeftIndent(10);
    style_heading->SetRightIndent(8);
    style_heading->SetParagraphSpacingBefore(10);
    style_heading->SetParagraphSpacingAfter(0);
    style_heading->SetCharacterStyleName("CharBase");
    //this->defParaHead->SetStyle(*style_heading);
    this->style_sheet->AddParagraphStyle(this->defParaHead);

    this->GetBuffer().SetStyleSheet(this->style_sheet.get());
    this->ApplyStyleSheet(this->style_sheet.get());

    new_document();
}


hmbRich::~hmbRich()
{
    // Буфер rich text хранит «сырой» указатель на style sheet и сам его не удаляет.
    // Перед уничтожением style_sheet явно обнуляем ссылку в буфере.
    this->GetBuffer().SetStyleSheet(nullptr);
}

void hmbRich::new_document()
{
    this->Clear();
    this->SetInsertionPoint(0);
    this->row_current = 0;
    this->row_total = 0;
}

void hmbRich::load_xml_handler()
{
    if (!wxRichTextBuffer::FindHandler(wxRICHTEXT_TYPE_XML))
    {
        wxRichTextBuffer::AddHandler(new wxRichTextXMLHandler);
    }
}


void hmbRich::save_file_as(void)
{
    wxFileDialog saveFileDialog(this, _("Save file as"), "", "",
                      "Plain text files (*.txt)|*.txt|Rich text XML (*.wxrt)|*.wxrt",
                       wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL) return;
    
    const int fileType = saveFileDialog.GetFilterIndex();
    wxFileName fileName(saveFileDialog.GetPath());
    if (fileName.GetExt().IsEmpty())
    {
        fileName.SetExt(fileType == 1 ? RICH_BUFFER_EXT : TEXT_BUFFER_EXT);
    }
    
    const wxString filePath = fileName.GetFullPath();

    if (fileType == 0) this->save_plain_file(filePath);
    if (fileType == 1) this->save_xml_file(filePath);

}

// --- Save the buffer content as plain text ---
void hmbRich::save_plain_file(const wxString filePath)
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

void hmbRich::save_xml_file(const wxString filePath)
{
    wxRichTextBuffer& buffer = this->GetBuffer();
    load_xml_handler();
    if (!buffer.SaveFile(filePath, wxRICHTEXT_TYPE_XML))
    {
        wxLogError(_("Cannot save rich buffer file '%s'."), filePath.wc_str());
    }
    return;
}

wxString hmbRich::get_buffer()
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

    return hmb_decode_xml(xml_text);
}

// --- Load the prepared XML data into the control's buffer ---
void hmbRich::push_xml_data(const wxString& content)
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
void hmbRich::load_file(const wxString filePath)
{
    if(filePath.IsEmpty()) return;
    if (!isFileExist(filePath)) return;

    HMB_FNAME = filePath;
    file_read(HMB_FNAME, HMB_SRC_DATA); // загрузить исходный текст в глобальную переменную

    auto fileName = wxFileName(filePath);
    wxString fileExt = fileName.GetExt();
    fileExt.LowerCase();

    new_document();

    if(fileExt == RICH_BUFFER_EXT) {
        this->push_xml_data(HMB_SRC_DATA);
    } else if(fileExt == MARK_BUFFER_EXT) {
        this->push_md_data();
    } else {
        this->push_plain_text();
    }
}

// --- Load the Markdown text ---
void hmbRich::push_md_data()
{
    cmark_node* node = cmark_parse_document(HMB_SRC_DATA.c_str(), HMB_SRC_DATA.size(), CMARK_OPT_DEFAULT);
    if (!node) {
        wxLogError(_("Error parsing file '%s'."), HMB_FNAME.wc_str());
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
    if (HMB_SRC_DATA.size() >= 1 && HMB_SRC_DATA.substr(HMB_SRC_DATA.size()-1) == "\n") append_line();
    cmark_node_free(node);
    this->EndSuppressUndo();

}

// --- Load the plain text content from a file ---
void hmbRich::push_plain_text()
{
    this->BeginSuppressUndo();
    this->WriteText(wxString::FromUTF8(HMB_SRC_DATA.c_str()));
    this->EndSuppressUndo();
    this->SetInsertionPoint(0);
}

// Переход на следующую строку.
void hmbRich::append_line() {
    Newline();
    this->row_current++;
}

void hmbRich::row_check(cmark_node* node) {
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
void hmbRich::show_literal(cmark_node* node) {
    const char* lit = cmark_node_get_literal(node);
    if (lit && *lit) this->WriteText(wxString::FromUTF8(lit));
}

void hmbRich::md_none(cmark_node* node) {
    this->WriteText("ERROR: Not found node\n");
}

void hmbRich::md_blockquote(cmark_node* node) {
    this->WriteText("Block quote\n");
}

void hmbRich::md_list(cmark_node* node) {
    this->WriteText("List\n");
}

void hmbRich::md_item(cmark_node* node) {
    this->WriteText("Item\n");
}

void hmbRich::md_code_block(cmark_node* node) {
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

void hmbRich::md_html_block(cmark_node* node) {
    this->WriteText("HTML block\n");
}
void hmbRich::md_custom_block(cmark_node* node) {
    this->WriteText("Custom block\n");
}

void hmbRich::md_header(cmark_node* node) {
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

void hmbRich::md_thematic_break(cmark_node* node) {
    this->WriteText("Thematic break\n");
}

void hmbRich::md_text(cmark_node* node) {
    show_literal(node);
}

void hmbRich::md_code_inline(cmark_node* node) {
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
void hmbRich::md_html_inline(cmark_node* node) {
    this->WriteText("HTML inline\n");
}
void hmbRich::md_custom_inline(cmark_node* node) {
    this->WriteText("Custom inline\n");
}
void hmbRich::md_emph(cmark_node* node) {
    this->BeginItalic();
    display_node(cmark_node_first_child(node));
    this->EndItalic();
}
void hmbRich::md_strong(cmark_node* node) {
    this->BeginBold();
    display_node(cmark_node_first_child(node));
    this->EndBold();
}
void hmbRich::md_link(cmark_node* node) {
    const char *url = cmark_node_get_url(node);
    //const char *title = cmark_node_get_title(node);
    //const char *text = cmark_node_get_literal(cmark_node_first_child(node));
    this->BeginURL(url, "style_urls");
    display_node(cmark_node_first_child(node));
    //this->WriteText(wxString::FromUTF8(text));
    this->EndURL();
}
void hmbRich::md_image(cmark_node* node) {
    this->WriteText("Image\n");
}
void hmbRich::md_unknown(cmark_node* node) {
    this->WriteText("Unknown\n");
}

void hmbRich::md_paragraph(cmark_node* node) {
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
void hmbRich::dbg_node(cmark_node* node, const char* info) {
    int start_line = 0;
    int end_line = 0;
    if (node) 
    { 
        start_line = cmark_node_get_start_line(node);
        end_line = cmark_node_get_end_line(node);
    }
    std::cerr << "[" << start_line << " - " << end_line << "] " << info << "\n";
}

void hmbRich::display_node(cmark_node* node)
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
    this->row_current = cmark_node_get_start_line(node);
    this->row_total = this->row_current + cmark_node_get_end_line(node) - 1;
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

wxMenu* hmbRich::edit_menu()
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


void hmbRich::on_change_font(wxCommandEvent& WXUNUSED(event))
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

void hmbRich::on_left_align(wxCommandEvent& WXUNUSED(event))
{
    wxTextAttr attr;
    attr.SetAlignment(wxTEXT_ALIGNMENT_LEFT);

    long start, end;
    this->GetSelection(& start, & end);
    this->SetStyle(start, end, attr);
}

void hmbRich::on_right_align(wxCommandEvent& WXUNUSED(event))
{
    wxTextAttr attr;
    attr.SetAlignment(wxTEXT_ALIGNMENT_RIGHT);

    long start, end;
    this->GetSelection(& start, & end);
    this->SetStyle(start, end, attr);
}

void hmbRich::OnJustify(wxCommandEvent& WXUNUSED(event))
{
    wxTextAttr attr;
    attr.SetAlignment(wxTEXT_ALIGNMENT_JUSTIFIED);

    long start, end;
    this->GetSelection(& start, & end);
    this->SetStyle(start, end, attr);
}

void hmbRich::OnCentre(wxCommandEvent& WXUNUSED(event))
{
    wxTextAttr attr;
    attr.SetAlignment(wxTEXT_ALIGNMENT_CENTRE);

    long start, end;
    this->GetSelection(& start, & end);
    this->SetStyle(start, end, attr);
}


void hmbRich::OnChangeTextColour(wxCommandEvent& WXUNUSED(event))
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

void hmbRich::OnChangeBackgroundColour(wxCommandEvent& WXUNUSED(event))
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

void hmbRich::OnLeftIndent(wxCommandEvent& WXUNUSED(event))
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

void hmbRich::OnRightIndent(wxCommandEvent& WXUNUSED(event))
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

void hmbRich::OnTabStops(wxCommandEvent& WXUNUSED(event))
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
