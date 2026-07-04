#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>

#include "rich.h"
#include "tree.h"
#include "tools.h"

// В документе может быть несколько вложенных wxRichTextBox. Возвращает индекс указанного.
static int find_box_index(wxRichTextParagraphLayoutBox& buffer, wxRichTextBox* target)
{
    if (!target) return -1;
    int index = 0;
    wxRichTextObjectList& children = buffer.GetChildren();
    for (auto it = children.begin(); it != children.end(); ++it)
    {
        wxRichTextObject* obj = *it;
        // Параграф может содержать вложенные объекты
        wxRichTextParagraph* para = dynamic_cast<wxRichTextParagraph*>(obj);
        if (para)
        {
            wxRichTextObjectList& paraChildren = para->GetChildren();
            for (auto jt = paraChildren.begin(); jt != paraChildren.end(); ++jt)
            {
                wxRichTextBox* box = dynamic_cast<wxRichTextBox*>(*jt);
                if (box)
                {
                    if (box == target) return index;
                    index++;
                }
            }
        }
        // Box может быть и непосредственным дочерним элементом буфера
        wxRichTextBox* box = dynamic_cast<wxRichTextBox*>(obj);
        if (box)
        {
            if (box == target) return index;
            index++;
        }
    }
    return -1;
}

// Поиск по индексу вложенного в документе объекта wxRichTextBox.
static wxRichTextBox* find_box_by_index(wxRichTextParagraphLayoutBox& buffer, int target_index)
{
    if (target_index < 0) return nullptr;
    int index = 0;
    wxRichTextObjectList& children = buffer.GetChildren();
    for (auto it = children.begin(); it != children.end(); ++it)
    {
        wxRichTextObject* obj = *it;
        wxRichTextParagraph* para = dynamic_cast<wxRichTextParagraph*>(obj);
        if (para)
        {
            wxRichTextObjectList& paraChildren = para->GetChildren();
            for (auto jt = paraChildren.begin(); jt != paraChildren.end(); ++jt)
            {
                wxRichTextBox* box = dynamic_cast<wxRichTextBox*>(*jt);
                if (box)
                {
                    if (index == target_index) return box;
                    index++;
                }
            }
        }
        wxRichTextBox* box = dynamic_cast<wxRichTextBox*>(obj);
        if (box)
        {
            if (index == target_index) return box;
            index++;
        }
    }
    return nullptr;
}


// Конструктор класса
hmbRich::hmbRich(wxWindow* parent)
    : wxRichTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
                    wxBORDER_NONE | wxWANTS_CHARS | wxVSCROLL)
{
    this->bind_events();
    this->init_styles();
    new_document();
}


hmbRich::~hmbRich()
{
    // Буфер rich text хранит «сырой» указатель на style sheet и сам его не удаляет.
    // Перед уничтожением style_sheet явно обнуляем ссылку в буфере.
    this->GetBuffer().SetStyleSheet(nullptr);
}


void hmbRich::init_styles()
{
    wxRichTextAttr rt_attr;
    wxTextBoxAttr& box_attr = rt_attr.GetTextBoxAttr();
    // --- Внутренние отступы области документа (padding) ---
    const int pad = this->FromDIP(8); // чтобы отступ адекватно выглядел на HiDPI
    box_attr.GetLeftPadding().SetValue(pad, wxTEXT_ATTR_UNITS_PIXELS);
    box_attr.GetRightPadding().SetValue(pad, wxTEXT_ATTR_UNITS_PIXELS);
    box_attr.GetTopPadding().SetValue(pad, wxTEXT_ATTR_UNITS_PIXELS);
    box_attr.GetBottomPadding().SetValue(pad, wxTEXT_ATTR_UNITS_PIXELS);
    // Шрифт по-умолчанию
    rt_attr.SetTextColour(HMB_COLOR_BASE_FG);
    rt_attr.SetBackgroundColour(HMB_COLOR_BASE_BG);
    rt_attr.SetFont(HMB_FONT_BASE);
    rt_attr.SetFontPointSize(HMB_FONT_BASE.GetPointSize());
    this->GetBuffer().SetAttributes(rt_attr);

    // Настройка вида блоков кода
    wxTextBoxAttr& cb_attr = this->code_block_style.GetTextBoxAttr();
    cb_attr.GetWidth().SetValue(100, wxTEXT_ATTR_UNITS_PERCENTAGE);
    cb_attr.GetRightMargin().SetValue(10, wxTEXT_ATTR_UNITS_POINTS);
    cb_attr.GetLeftPadding().SetValue(10, wxTEXT_ATTR_UNITS_POINTS);
    cb_attr.GetRightPadding().SetValue(10, wxTEXT_ATTR_UNITS_POINTS);
    this->code_block_style.SetFlags(wxTEXT_ATTR_FONT | wxTEXT_ATTR_TEXT_COLOUR | wxTEXT_ATTR_BACKGROUND_COLOUR);
    this->code_block_style.SetFont(HMB_FONT_MONO);
    this->code_block_style.SetTextColour(this->color_code_fg);
    this->code_block_style.SetBackgroundColour(this->color_gray_bg);
}

void hmbRich::bind_events()
{
    this->Bind(wxEVT_MOTION, [this](wxMouseEvent& event)
    {
        long pos = wxNOT_FOUND;
        wxString url = wxEmptyString;
        wxTextAttr attr;

        this->HitTest(event.GetPosition(), &pos);
        if (pos != wxNOT_FOUND && this->GetStyle(pos, attr) && attr.HasFlag(wxTEXT_ATTR_URL))
        {
            url = attr.GetURL();
        }

        this->show_url(url);
        event.Skip();
    });

    this->Bind(wxEVT_LEAVE_WINDOW, [this](wxMouseEvent& event)
    {
        this->show_url(wxEmptyString);
        event.Skip();
    });

    // Клик по гиперссылке — показать адрес во всплывающем окне.
    this->Bind(wxEVT_LEFT_UP, [this](wxMouseEvent& event)
    {
        long pos = wxNOT_FOUND;
        wxTextAttr attr;

        this->HitTest(event.GetPosition(), &pos);
        if (pos != wxNOT_FOUND && this->GetStyle(pos, attr) && attr.HasFlag(wxTEXT_ATTR_URL))
        {
            wxString url = attr.GetURL();
            if (!url.IsEmpty())
            {
                wxMessageBox(url, "URL", wxOK | wxICON_INFORMATION, this);
            }
        }
        event.Skip();
    });

    // Потеря объектом фокуса
    this->Bind(wxEVT_KILL_FOCUS, [this](wxFocusEvent& event)
    {
        this->cursor_position_save();
        event.Skip();
    });
}


void hmbRich::cursor_position_save()
{
    // Сохранить позицию прокрутки текста в окне
    this->cursor_position.scroll_y = this->GetScrollPos(wxVERTICAL);

    // Определить, находится ли фокус внутри вложенного объекта
    wxRichTextParagraphLayoutBox* focus_obj = this->GetFocusObject();
    this->cursor_position.focus_in_object = (focus_obj != nullptr && focus_obj != &this->GetBuffer());

    long caret_pos = this->GetInsertionPoint();

    if (this->cursor_position.focus_in_object)
    {
        // Сохраняем локальную строку/колонку внутри box
        this->PositionToXY(caret_pos, &this->cursor_position.caret_col, &this->cursor_position.caret_line);

        // Определяем порядковый номер box-а в документе
        wxRichTextBox* box = nullptr;
        wxRichTextObject* obj = dynamic_cast<wxRichTextObject*>(focus_obj);
        while (obj && !box)
        {
            box = dynamic_cast<wxRichTextBox*>(obj);
            obj = obj->GetParent();
        }
        this->cursor_position.box_index = find_box_index(this->GetBuffer(), box);
    }
    else
    {
        // Обычный случай — позиция в основном буфере
        this->PositionToXY(caret_pos, &this->cursor_position.caret_col, &this->cursor_position.caret_line);
        this->cursor_position.box_index = -1;
    }

}

void hmbRich::new_document()
{
    // Снять фокус, чтобы исключить блокировку элементов
    this->SetFocusObject(&this->GetBuffer(), false);

    // Очистить стили
    this->GetBuffer().EndAllStyles();
    this->SetDefaultStyle(wxTextAttr());
    this->Newline();
    
    this->Clear();
    this->SetInsertionPoint(0);
    this->row_current = 0;
    this->row_total = 0;
}


// чтение данных из буфера в UTF-8 строку в виде XML.
void hmbRich::read_buffer_xml(std::string &out)
{
    out.clear();
    wxRichTextBuffer& buffer = this->GetBuffer();

    if (!wxRichTextBuffer::FindHandler(wxRICHTEXT_TYPE_XML))
    {
        wxRichTextBuffer::AddHandler(new wxRichTextXMLHandler);
    }

    wxString xml_text;
    wxStringOutputStream xml_stream(&xml_text);
    if (!buffer.SaveFile(xml_stream, wxRICHTEXT_TYPE_XML))
    {
        wxLogWarning(_("Cannot serialize rich buffer to XML string."));
        return;
    }
    wx_to_utf8(hmb_decode_xml(xml_text), out);
}

// Привязка статус-бара для отображения URL при наведении мыши на ссылку.
void hmbRich::bind_subscriber(hmbStatusBar* status_bar)
{
    this->subscriber = status_bar;
}

// Отображение URL в статус-баре при наведении мыши на ссылку.
void hmbRich::show_url(const wxString& url) {
    if (this->subscriber) {
        this->subscriber->show_url(url);
    }
}

// Вствка нового абзаца
void hmbRich::new_line() {
    this->Newline();
    this->row_current++;
}

// Вставка мягкого переноса строки
// BUG: wxRichTextCtrl отображает символ "SOFTBREAK" (код 29) не стабильно -
// сети в строке есть форагменты инлайн форматирования, то строка не разделяется
// как ожидается. Если таких элементов нет - строка разделяется. Для стабилизации
// вызов LineBreak() заменен вводом переноса строки.
void hmbRich::line_break() {
    //this->LineBreak();
    if(this->show_softbreak) this->WriteText("\n");
    else this->WriteText(" ");
    this->row_current++;
}

// В cmark пустые строки между block-нодами не представлены отдельными узлами.
// Восстанавливаем их по source positions текущей block-ноды.
void hmbRich::row_check(cmark_node* node) {
    if(!node) return;
    int row_begin = cmark_node_get_start_line(node);
    while (row_begin > this->row_current)
    {
        this->new_line();
    }
}

void hmbRich::md_none(cmark_node* node) {
    this->WriteText("ERROR: Not found node\n");
}

void hmbRich::md_blockquote(cmark_node* node) {
    this->WriteText("Block quote\n");
}


void hmbRich::md_num_list(cmark_node* node) {
    int d = 40;
    int n_start = cmark_node_get_list_start(node);
    cmark_node* item = cmark_node_first_child(node);
    int indent = d + this->list_depth * d;
    int sub_indent = indent * (digits(n_start) - 1) + 50;
    this->list_depth++;
    while (item && cmark_node_get_type(item) == CMARK_NODE_ITEM)
    {
        this->BeginNumberedBullet(n_start++, indent, sub_indent,
            wxTEXT_ATTR_BULLET_STYLE_ARABIC|wxTEXT_ATTR_BULLET_STYLE_PERIOD);
        this->node_iterator(item);
        this->EndNumberedBullet();
        item = cmark_node_next(item);
    }
    this->list_depth--;
}


void hmbRich::md_bul_list(cmark_node* node) {
    int d = 40;
    static enum wxTextAttrBulletStyle s = wxTEXT_ATTR_BULLET_STYLE_STANDARD;
    cmark_node* item = cmark_node_first_child(node);
    int indent = d + this->list_depth * d;
    this->list_depth++;
    this->BeginStandardBullet("-", indent, d, s);
    while (item && cmark_node_get_type(item) == CMARK_NODE_ITEM)
    {
        this->node_iterator(item);
        item = cmark_node_next(item);
    }
    this->EndStandardBullet();
    this->list_depth--;
}


void hmbRich::md_list(cmark_node* node) {
    if (!node) return;
    if(cmark_node_get_list_type(node) == CMARK_ORDERED_LIST) this->md_num_list(node);
    else this->md_bul_list(node);
}

void hmbRich::md_item(cmark_node* node) {
    this->WriteText("Item\n");
}

void hmbRich::md_code_block(cmark_node* node) {
    wxRichTextBox* box = this->WriteTextBox(this->code_block_style);
    const char* lit = cmark_node_get_literal(node);
    std::istringstream ss(lit);
    std::string line;
    while (std::getline(ss, line)) {
        box->AddParagraph(wxString::FromUTF8(line.c_str()));
        this->row_current++;
    }
    box->GetBuffer()->EndAllStyles();
    box->AddParagraph(wxEmptyString);
    this->row_current++;
}

void hmbRich::md_html_block(cmark_node* node) {
    this->WriteText("HTML block\n");
}
void hmbRich::md_custom_block(cmark_node* node) {
    this->WriteText("Custom block\n");
}

void hmbRich::md_header(cmark_node* node) {
    int font_size = 18 - cmark_node_get_heading_level(node) * 2;

    this->BeginFontSize(font_size);
    this->BeginTextColour("#404080");
    this->BeginBold();
    
    this->node_iterator(node);
    
    this->EndBold();
    this->EndTextColour();
    this->EndFontSize();

    this->new_line();
}

void hmbRich::md_thematic_break(cmark_node* node) {
    this->WriteText("Thematic break\n");
}

// Содержимое текстовых узлов, code, html_inline и т.д.
void hmbRich::md_text(cmark_node* node) {
    const char* lit = cmark_node_get_literal(node);
    if (lit && *lit) this->WriteText(wxString::FromUTF8(lit));
}

void hmbRich::md_code_inline(cmark_node* node) {
    wxRichTextAttr codeAttr;
    codeAttr.SetFontFaceName(HMB_FONT_MONO.GetFaceName());
    codeAttr.SetTextColour(this->color_code_fg);
    codeAttr.SetBackgroundColour(this->color_gray_bg);
    this->BeginStyle(codeAttr);
    this->WriteText("\'");
    this->md_text(node);
    this->WriteText("\'");
    this->EndStyle();
}

void hmbRich::md_html_inline(cmark_node* node) {
    this->WriteText("HTML inline\n");
}

void hmbRich::md_custom_inline(cmark_node* node) {
    this->WriteText("Custom inline\n");
}

void hmbRich::md_emph(cmark_node* node) {
    this->BeginItalic();
    this->node_iterator(node);
    this->EndItalic();
}

void hmbRich::md_strong(cmark_node* node) {
    this->BeginBold();
    this->node_iterator(node);
    this->EndBold();
}

void hmbRich::md_link(cmark_node* node) {
    const char *url = cmark_node_get_url(node);
    wxRichTextAttr urlAttr;
    urlAttr.SetTextColour(this->color_urls_fg);
    urlAttr.SetFontUnderlined(true);
    urlAttr.SetURL(wxString::FromUTF8(url));
    this->BeginStyle(urlAttr);
    this->node_iterator(node);
    this->EndStyle();
}

void hmbRich::md_image(cmark_node* node) {
    const char* image_url = cmark_node_get_url(node);
    if (!image_url || !*image_url) return;
    
    auto filepath = wxString::FromUTF8(image_url);
    filepath.Replace("\\", "/");
    if (filepath.StartsWith("/")) filepath = filepath.Mid(1);

    auto base_dir = wxFileName(HMB_FNAME).GetPath();
    base_dir.Replace("\\", "/");
    if (!base_dir.EndsWith("/")) base_dir += "/";

    filepath = base_dir + filepath;

    wxImage img(filepath, wxBITMAP_TYPE_ANY); // Вставка PNG
    if (img.IsOk()) this->WriteImage(wxBitmap(img));
}

void hmbRich::md_unknown(cmark_node* node) {
    this->WriteText("Unknown node\n");
}

// Стандартный абзац.
void hmbRich::md_paragraph(cmark_node* node) {
    this->node_iterator(node);
    this->new_line();
}

void hmbRich::node_dispatcher(cmark_node* node)
{

  //debug_node(node);  //!!DEBUG!!
  //return;  //!!DEBUG!!

  if (!node) return;
  cmark_node_type t = cmark_node_get_type(node);

  switch (t) {
  case CMARK_NODE_NONE:
    md_none(node);
    break;
  // -- Block nodes --
  case CMARK_NODE_HEADING:
    md_header(node);
    break;
  case CMARK_NODE_PARAGRAPH:
    md_paragraph(node);
    break;
  case CMARK_NODE_BLOCK_QUOTE:
    md_blockquote(node);
    break;
  case CMARK_NODE_LIST:
    md_list(node);
    break;
  case CMARK_NODE_ITEM:
    md_item(node);
    break;
  case CMARK_NODE_CODE_BLOCK:
    md_code_block(node);
    break;
  case CMARK_NODE_HTML_BLOCK:
    md_html_block(node);
    break;
  case CMARK_NODE_CUSTOM_BLOCK:
    md_custom_block(node);
    break;
  case CMARK_NODE_THEMATIC_BREAK:
    md_thematic_break(node);
    break;
  // -- Inline nodes --
  case CMARK_NODE_TEXT:
    md_text(node);
    break;
  case CMARK_NODE_SOFTBREAK:
    this->line_break();
    break;
  case CMARK_NODE_LINEBREAK:
    this->line_break();
    break;
  case CMARK_NODE_CODE:
    md_code_inline(node);
    break;
  case CMARK_NODE_HTML_INLINE:
    md_html_inline(node);
    break;
  case CMARK_NODE_CUSTOM_INLINE:
    md_custom_inline(node);
    break;
  case CMARK_NODE_EMPH:
    md_emph(node);
    break;
  case CMARK_NODE_STRONG:
    md_strong(node);
    break;
  case CMARK_NODE_LINK:
    md_link(node);
    break;
  case CMARK_NODE_IMAGE:
    md_image(node);
    break;
  default:
    md_unknown(node);
    break;
  }
}

// Последовательный обход ветвей дерева нод
void hmbRich::node_iterator(cmark_node* node)
{
    if (!node) return;
    cmark_node* child = cmark_node_first_child(node);
    while (child) {
        if (cmark_node_is_block(child)) row_check(child);
        node_dispatcher(child);
        child = cmark_node_next(child);
    }
}

// --- Load the Markdown text ---
void hmbRich::load_src_data()
{
    cmark_node* node = cmark_parse_document(HMB_SRC_DATA.c_str(), HMB_SRC_DATA.size(), CMARK_OPT_DEFAULT);
    if(cmark_node_get_type(node) != CMARK_NODE_DOCUMENT)
    {
        wxLogError(_("Markdown format error."));
        if (node) cmark_node_free(node);
        this->load_as_plain_text();
        return;
    }

    new_document();
    this->row_current = cmark_node_get_start_line(node);
    this->row_total = this->row_current + cmark_node_get_end_line(node) - 1;
    this->Freeze();
    this->BeginSuppressUndo();
    node_iterator(node);
    
    // Защита от утечки стека стилей: все Begin...() должны быть закрыты End...()
    size_t stack_size = this->GetBuffer().GetStyleStackSize();
    if (stack_size > 0)
    {
        wxLogWarning(_("Style stack leak: %zu unclosed Begin...() calls after rendering."),
                     stack_size);
        this->GetBuffer().EndAllStyles();
    }

    // Дополнить пустые строки до конца документа.
    while (this->row_current < this->row_total) {
        this->new_line();
    }
    
    // BUG: Парсер игнорит в файле завешающий '\n'.
    // FIX: Добавление последней строки, если присутствует завешающий '\n'.
    if (HMB_SRC_DATA.size() >= 1 && HMB_SRC_DATA.substr(HMB_SRC_DATA.size()-1) == "\n")
        this->new_line();
    
    cmark_node_free(node);
    this->EndSuppressUndo();
    this->Thaw();

    cursor_position_load();

    // DEBUG
    //std::string buffer_content = "";
    //this->debug_buffer_content(buffer_content);
    //std::cout  << buffer_content;
}


// Восстановление позиции курсора
void hmbRich::cursor_position_load()
{
     // Если курсор находился на строке во вложенном wxRichTextBox
    if (this->cursor_position.focus_in_object && this->cursor_position.box_index >= 0)
    {
        // Ищем wxRichTextBox по сохранённому порядковому номеру
        wxRichTextBox* box = find_box_by_index(this->GetBuffer(), this->cursor_position.box_index);
        if (box)
        {
            this->SetFocusObject(box, false);

            long restore_pos = this->XYToPosition(this->cursor_position.caret_col, this->cursor_position.caret_line);
            if (restore_pos != -1)
            {
                this->SetInsertionPoint(restore_pos);
            }

            // Восстановить позицию прокрутки
            this->SetScrollPos(wxVERTICAL, this->cursor_position.scroll_y);
            this->Refresh();
            return;
        }
    }

    // Обычный случай — восстановление в основном буфере
    long restore_pos = this->XYToPosition(this->cursor_position.caret_col, this->cursor_position.caret_line);
    if (restore_pos != -1)
    {
        this->SetInsertionPoint(restore_pos);
    }

    // Восстановить позицию прокрутки
    this->SetScrollPos(wxVERTICAL, this->cursor_position.scroll_y);
    this->Refresh();
}


void hmbRich::load_as_plain_text()
{
    wxLogError(_("Error parsing file '%s'."), HMB_FNAME.wc_str());
    this->Freeze();
    new_document();
    this->BeginSuppressUndo();
    this->WriteText(wxString::FromUTF8(HMB_SRC_DATA.c_str()));
    this->EndSuppressUndo();
    this->Thaw();
    cursor_position_load();
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

// DEBUG: Обход внутренней структуры буфера wxRichTextCtrl
void hmbRich::debug_buffer_content(std::string& out)
{
    wxRichTextBuffer& buffer = this->GetBuffer();
    std::ostringstream ss;

    for (auto it = buffer.GetChildren().begin(); it != buffer.GetChildren().end(); ++it)
    {
        // Указатель на абзац — дочерний элемент верхнего уровня буфера
        wxRichTextParagraph* para = dynamic_cast<wxRichTextParagraph*>(*it);
        if (!para) continue;

        ss << "[PARA range=" << para->GetRange().GetStart()
           << ".." << para->GetRange().GetEnd() << "]\n";

        for (auto jt = para->GetChildren().begin(); jt != para->GetChildren().end(); ++jt)
        {
            wxRichTextObject* obj = *jt;

            // Приведение объекта к типу текстового фрагмента
            wxRichTextPlainText* txt = dynamic_cast<wxRichTextPlainText*>(obj);
            if (txt)
            {
                // Текстовое содержимое фрагмента
                wxString text = txt->GetText();
                ss << "  TEXT [" << text.utf8_string() << "] len=" << text.length();
                // Вывод кодов каждого символа
                ss << " codes:";
                for (size_t i = 0; i < text.length(); i++)
                    ss << " " << (int)text[i];

                // Атрибуты стиля текстового фрагмента (жирность, курсив и т.д.)
                wxRichTextAttr attr = txt->GetAttributes();
                if (attr.GetFontWeight() == wxFONTWEIGHT_BOLD)
                    ss << " [BOLD]";
                if (attr.GetFontStyle() == wxFONTSTYLE_ITALIC)
                    ss << " [ITALIC]";
                ss << "\n";
            }
            else
            {
                ss << "  OBJ type=" << wxString(obj->GetClassInfo()->GetClassName()).utf8_string() << "\n";
            }
        }
    }
    out = ss.str(); // полный дамп структуры буфера в виде UTF-8 строки
}