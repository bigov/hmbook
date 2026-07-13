#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>

#include "rich.h"
#include "panel_tree.h"

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

    // Стиль таблицы
    this->table_attr.GetTextBoxAttr().GetWidth().SetValue(100, wxTEXT_ATTR_UNITS_PERCENTAGE);

    //this->table_attr.GetTextBoxAttr().GetBorder().GetTop().SetStyle(wxTEXT_BOX_ATTR_BORDER_THIN);
    //this->table_attr.GetTextBoxAttr().GetBorder().GetTop().SetColour(*wxBLACK);
    this->table_attr.GetTextBoxAttr().GetBorder().GetTop().SetWidth(0, wxTEXT_ATTR_UNITS_PIXELS);

    //this->table_attr.GetTextBoxAttr().GetBorder().GetRight().SetStyle(wxTEXT_BOX_ATTR_BORDER_THIN);
    //this->table_attr.GetTextBoxAttr().GetBorder().GetRight().SetColour(*wxBLACK);
    this->table_attr.GetTextBoxAttr().GetBorder().GetRight().SetWidth(0, wxTEXT_ATTR_UNITS_PIXELS);

    //this->table_attr.GetTextBoxAttr().GetBorder().GetBottom().SetStyle(wxTEXT_BOX_ATTR_BORDER_THIN);
    //this->table_attr.GetTextBoxAttr().GetBorder().GetBottom().SetColour(*wxBLACK);
    this->table_attr.GetTextBoxAttr().GetBorder().GetBottom().SetWidth(0, wxTEXT_ATTR_UNITS_PIXELS);
    
    //this->table_attr.GetTextBoxAttr().GetBorder().GetLeft().SetStyle(wxTEXT_BOX_ATTR_BORDER_THIN);
    //this->table_attr.GetTextBoxAttr().GetBorder().GetLeft().SetColour(*wxBLACK);
    this->table_attr.GetTextBoxAttr().GetBorder().GetLeft().SetWidth(0, wxTEXT_ATTR_UNITS_PIXELS);

    // Стиль ячеек
    this->table_cell_attr.GetTextBoxAttr().GetPadding().GetLeft().SetValue(4, wxTEXT_ATTR_UNITS_PIXELS);
    this->table_cell_attr.GetTextBoxAttr().GetPadding().GetRight().SetValue(4, wxTEXT_ATTR_UNITS_PIXELS);
    this->table_cell_attr.GetTextBoxAttr().GetPadding().GetTop().SetValue(2, wxTEXT_ATTR_UNITS_PIXELS);
    this->table_cell_attr.GetTextBoxAttr().GetPadding().GetBottom().SetValue(2, wxTEXT_ATTR_UNITS_PIXELS);
    //this->table_cell_attr.GetTextBoxAttr().GetBorder().GetTop().SetStyle(wxTEXT_BOX_ATTR_BORDER_THIN);
    //this->table_cell_attr.GetTextBoxAttr().GetBorder().GetBottom().SetStyle(wxTEXT_BOX_ATTR_BORDER_THIN);
    //this->table_cell_attr.GetTextBoxAttr().GetBorder().GetLeft().SetStyle(wxTEXT_BOX_ATTR_BORDER_THIN);
    //this->table_cell_attr.GetTextBoxAttr().GetBorder().GetRight().SetStyle(wxTEXT_BOX_ATTR_BORDER_THIN);
    this->table_cell_attr.GetTextBoxAttr().GetBorder().GetTop().SetColour(*wxLIGHT_GREY);
    this->table_cell_attr.GetTextBoxAttr().GetBorder().GetBottom().SetColour(*wxLIGHT_GREY);
    this->table_cell_attr.GetTextBoxAttr().GetBorder().GetLeft().SetColour(*wxLIGHT_GREY);
    this->table_cell_attr.GetTextBoxAttr().GetBorder().GetRight().SetColour(*wxLIGHT_GREY);
    this->table_cell_attr.GetTextBoxAttr().GetBorder().GetTop().SetWidth(1, wxTEXT_ATTR_UNITS_PIXELS);
    this->table_cell_attr.GetTextBoxAttr().GetBorder().GetBottom().SetWidth(1, wxTEXT_ATTR_UNITS_PIXELS);
    this->table_cell_attr.GetTextBoxAttr().GetBorder().GetLeft().SetWidth(1, wxTEXT_ATTR_UNITS_PIXELS);
    this->table_cell_attr.GetTextBoxAttr().GetBorder().GetRight().SetWidth(1, wxTEXT_ATTR_UNITS_PIXELS);
}

void hmbRich::bind_events()
{
    this->Bind(wxEVT_MOTION, [this](wxMouseEvent& event)
    {
        long pos = wxNOT_FOUND;
        this->HitTest(event.GetPosition(), &pos);
        if (pos == wxNOT_FOUND) return;

        wxTextAttr attr;
        this->GetStyle(pos, attr);
        if (attr.HasFlag(wxTEXT_ATTR_URL)) this->show_url(attr.GetURL());

        event.Skip();
    });

    this->Bind(wxEVT_LEAVE_WINDOW, [this](wxMouseEvent& event)
    {
        this->show_url(wxEmptyString);
        event.Skip();
    });

    // Клик по гиперссылке.
    this->Bind(wxEVT_LEFT_UP, [this](wxMouseEvent& event)
    {
        long pos = wxNOT_FOUND;
        this->HitTest(event.GetPosition(), &pos);
        if (pos == wxNOT_FOUND) return;
        
        wxTextAttr attr;
        this->GetStyle(pos, attr);
        if (attr.HasFlag(wxTEXT_ATTR_URL)) this->left_click_url(attr.GetURL());
        event.Skip();
    });

    // Потеря объектом фокуса
    this->Bind(wxEVT_KILL_FOCUS, [this](wxFocusEvent& event)
    {
        this->cursor_position_save();
        event.Skip();
    });

    // чувствительность прокрутки колеса мыши
    this->Bind(wxEVT_MOUSEWHEEL, [this](wxMouseEvent& event)
    {
    int rotation = event.GetWheelRotation();
    int delta = event.GetWheelDelta();
    
    // Множитель чувствительности: <1.0 — менее чувствительно, >1.0 — более
    double sensitivity = 5.0;
    
    int lines = static_cast<int>((rotation / delta) * event.GetLinesPerAction() * sensitivity);
    if (lines == 0 && rotation != 0)
        lines = (rotation > 0) ? 1 : -1;
    
    this->ScrollLines(-lines);
    // Не вызываем event.Skip(), чтобы подавить стандартную обработку
    });
}


void hmbRich::cursor_position_save()
{
    this->cursor_position.box_index = -1;
    
    // Проверить, находится ли фокус внутри вложенного объекта
    auto focus_obj = this->GetFocusObject();
    this->cursor_position.focus_in_object = (focus_obj != nullptr && focus_obj != &this->GetBuffer());
    if (this->cursor_position.focus_in_object)
    {
        wxRichTextBox* box = nullptr;
        wxRichTextObject* obj = dynamic_cast<wxRichTextObject*>(focus_obj);
        while (obj && !box)
        {
            box = dynamic_cast<wxRichTextBox*>(obj);
            obj = obj->GetParent();
        }
        this->cursor_position.box_index = find_box_index(this->GetBuffer(), box);
    }

    this->cursor_position.scroll_y = this->GetScrollPos(wxVERTICAL);    
    long caret_pos = this->GetInsertionPoint();
    this->PositionToXY(caret_pos, &this->cursor_position.caret_col, &this->cursor_position.caret_line);
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
void hmbRich::bind_status_bar(hmbStatusBar* status_bar)
{
    this->status_bar_ptr = status_bar;
}

// Отображение URL в статус-баре при наведении мыши на ссылку.
void hmbRich::show_url(const wxString& url) {
    if (this->status_bar_ptr) {
        this->status_bar_ptr->show_url(url);
    }
}

// Приязка указателя на панель навигации требуется для реализации переходов по ссылкам
void hmbRich::bind_paneltree_ptr(hmbPanelTree* panel_tree)
{
    this->panel_tree_ptr = panel_tree;
}


void hmbRich::follow_local_page(const wxString url)
{
    if (!this->panel_tree_ptr) return;
    wxString fpath = url;
    fpath.Replace("/", wxFileName::GetPathSeparator());
    wxString fullPath = this->panel_tree_ptr->get_current_dir() + wxFileName::GetPathSeparator() + fpath;
    this->panel_tree_ptr->set_cursor_to(fullPath);
}


void hmbRich::left_click_url(wxString url)
{
    if (url.IsEmpty()) return;
    if (url.EndsWith(".md"))
    {
        follow_local_page(url);
        return;
    }

    if (url.StartsWith("http://")) 
    {
        wxLaunchDefaultBrowser(url);
        return;
    }

    if (url.StartsWith("https://")) 
    {
        wxLaunchDefaultBrowser(url);
        return;
    }

    wxMessageBox(url, "URL", wxOK | wxICON_INFORMATION, this);
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
    if(this->ctable) return;
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
    wxString url = wxString::FromUTF8(cmark_node_get_url(node));
    wxRichTextAttr urlAttr;
    if (url.EndsWith(".md"))urlAttr.SetTextColour(this->color_url_local);
    else urlAttr.SetTextColour(this->color_url_network);
    urlAttr.SetFontUnderlined(true);
    urlAttr.SetURL(url);
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

    wxImage img(filepath, wxBITMAP_TYPE_ANY);
    if (img.IsOk()) this->WriteImage(wxBitmap(img));
    else wxLogError(_("Cannot open file '%s'."), filepath.wc_str());
}

void hmbRich::md_unknown(cmark_node* node) {
    this->WriteText("Unknown node\n");
}

// Стандартный абзац.
void hmbRich::md_paragraph(cmark_node* node) {
    this->node_iterator(node);
    if(this->row_current < this->row_total)
        this->new_line();
}

void hmbRich::md_table(cmark_node* node) {
    wxRichTextTable* table = new wxRichTextTable;
    //table->SetBasicStyle(this->GetBasicStyle());
    table->SetAttributes(this->table_attr);
    uint16_t col_count = cmark_gfm_extensions_get_table_columns(node);
    table->CreateTable(0, col_count);
    wxRichTextObject* obj = this->GetFocusObject()->InsertObjectWithUndo(
                &this->GetBuffer(), this->GetInsertionPoint(), table, this,
                wxRICHTEXT_INSERT_WITH_PREVIOUS_PARAGRAPH_STYLE);
    this->ctable = wxDynamicCast(obj, wxRichTextTable);
    this->ctable_row = 0;
    this->node_iterator(node);                       // Обход строк таблицы
    this->SetFocusObject(&this->GetBuffer(), false); // восстановить фокус на основной буфер

    // Переместить позицию вставки за таблицу, чтобы последующий контент
    // документа не оказался перед ней
    if (this->ctable) {
        long after_table = this->ctable->GetRange().GetEnd() + 1;
        this->SetInsertionPoint(after_table);
    }

    this->row_current += this->ctable_row;
    this->ctable = nullptr;
    this->ctable_row = 0;
    this->ctable_col = 0;
}

void hmbRich::md_table_row(cmark_node* node) {
     if (!this->ctable) return;

    this->ctable->AddRows(this->ctable->GetRowCount(), 1, this->table_cell_attr);
    this->ctable_col = 0;
    bool is_header = cmark_gfm_extensions_get_table_row_is_header(node);
    if (is_header) this->BeginBold();
    this->node_iterator(node);
    if (is_header) this->EndBold();
    this->ctable_row++;
}

void hmbRich::md_table_cell(cmark_node* node) {
    if (!this->ctable) return;
    if (this->ctable_col >= this->ctable->GetColumnCount()) return;

    wxRichTextCell* cell = this->ctable->GetCell(this->ctable_row, this->ctable_col);
    if (!cell) { this->ctable_col++; return; }
    //cell->SetAttributes(this->table_cell_attr); // Применить стиль ячейки
    // Переключить фокус записи на ячейку и записать содержимое
    this->SetFocusObject(cell, false);
    cell->Clear();
    this->node_iterator(node);
    this->ctable_col++;
}

void hmbRich::node_dispatcher(cmark_node* node)
{

  //debug_node(node);  //!!DEBUG!!

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
    // Типы CMARK_NODE_TABLE, CMARK_NODE_TABLE_ROW, CMARK_NODE_TABLE_CELL — 
    // это глобальные переменные (не enum-константы), поэтому их нельзя
    // использовать в case, но можно в цепочке if внутри default:
    if (t == CMARK_NODE_TABLE) { md_table(node); break; }
    if (t == CMARK_NODE_TABLE_ROW) { md_table_row(node); break; }
    if (t == CMARK_NODE_TABLE_CELL) { md_table_cell(node); break; }
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
        if ((cmark_node_get_type(child) & CMARK_NODE_TYPE_MASK) == CMARK_NODE_TYPE_BLOCK)
            row_check(child);
        node_dispatcher(child);
        child = cmark_node_next(child);
    }
}

// --- Load the Markdown text ---
void hmbRich::load_src_data()
{
    auto md_tree = MdTree(HMB_SRC_DATA);
    if(md_tree.error())
    {
         this->load_as_plain_text();
         return;
    }

    new_document();
    this->row_current = md_tree.start_line();
    this->row_total = this->row_current + md_tree.end_line() - 1;
    this->Freeze();
    this->BeginSuppressUndo();
    node_iterator(md_tree.node_ptr);
    this->EndSuppressUndo();
    this->Thaw();
    cursor_position_load();
}


// Восстановление позиции курсора
void hmbRich::cursor_position_load()
{
    // Если курсор находился на строке во вложенном wxRichTextBox
    if (this->cursor_position.focus_in_object && this->cursor_position.box_index >= 0)
    {
         wxRichTextBox* box = find_box_by_index(this->GetBuffer(), this->cursor_position.box_index);
        if (box) this->SetFocusObject(box, false);
    }

    long restore_pos = this->XYToPosition(this->cursor_position.caret_col, this->cursor_position.caret_line);
    if (restore_pos != -1) this->SetInsertionPoint(restore_pos);
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
