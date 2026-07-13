#include "tools.h"
#include "md_parser.h"

// ---------------------------------------------
// --- диапазон строк ноды (для печати отладки) ---
static const char* cmark_type_to_const_name(cmark_node_type t)
{
    switch (t)
    {
    case CMARK_NODE_NONE: return "NONE";
    case CMARK_NODE_DOCUMENT: return "DOCUMENT";
    case CMARK_NODE_BLOCK_QUOTE: return "BLOCK_QUOTE";
    case CMARK_NODE_LIST: return "LIST";
    case CMARK_NODE_ITEM: return "ITEM";
    case CMARK_NODE_CODE_BLOCK: return "CODE_BLOCK";
    case CMARK_NODE_HTML_BLOCK: return "HTML_BLOCK";
    case CMARK_NODE_CUSTOM_BLOCK: return "CUSTOM_BLOCK";
    case CMARK_NODE_PARAGRAPH: return "PARAGRAPH";
    case CMARK_NODE_HEADING: return "HEADING";
    case CMARK_NODE_THEMATIC_BREAK: return "THEMATIC_BREAK";
    case CMARK_NODE_TEXT: return "TEXT";
    case CMARK_NODE_SOFTBREAK: return "SOFTBREAK";
    case CMARK_NODE_LINEBREAK: return "LINEBREAK";
    case CMARK_NODE_CODE: return "CODE";
    case CMARK_NODE_HTML_INLINE: return "HTML_INLINE";
    case CMARK_NODE_CUSTOM_INLINE: return "CUSTOM_INLINE";
    case CMARK_NODE_EMPH: return "EMPH";
    case CMARK_NODE_STRONG: return "STRONG";
    case CMARK_NODE_LINK: return "LINK";
    case CMARK_NODE_IMAGE: return "IMAGE";
    default: return "UNKNOWN";
    }
}

void debug_node(cmark_node* node) {
    int start_line = 0;
    int end_line = 0;
    cmark_node_type t = CMARK_NODE_NONE;
    if (node) 
    { 
        start_line = cmark_node_get_start_line(node);
        end_line = cmark_node_get_end_line(node);
        t = cmark_node_get_type(node);
    }
    std::cerr << "[" << start_line << " - " << end_line << "] " << cmark_type_to_const_name(t) << "\n";
}

MdTree::MdTree(const std::string &text)
{
    // Регистрация расширений GFM
    cmark_gfm_core_extensions_ensure_registered();

    // Создание парсера с подключением расширения "table"
    cmark_parser* parser = cmark_parser_new(CMARK_OPT_DEFAULT);
    cmark_syntax_extension* table_ext = cmark_find_syntax_extension("table");
    if (table_ext) cmark_parser_attach_syntax_extension(parser, table_ext);

    cmark_parser_feed(parser, text.c_str(), text.size());
    this->tree_ptr.reset(cmark_parser_finish(parser));
    cmark_parser_free(parser);

    if(!this->tree_ptr || cmark_node_get_type(this->tree_ptr.get()) != CMARK_NODE_DOCUMENT)
    {
        wxLogError(_("Markdown format error."));
        if (this->tree_ptr) cmark_node_free(this->tree_ptr.get());
        this->tree_ptr = nullptr;
    } else {
        this->end_row_num = cmark_node_get_end_line(this->tree_ptr.get());
        
        // BUG: cmark игнорит в файле завершающий '\n'.
        // FIX: если присутствует завешающий '\n', то добавить параграф
        if (text.size() >= 1 && text.substr(text.size()-1) == "\n")
        {
            cmark_node* empty_row = cmark_node_new(CMARK_NODE_PARAGRAPH);
            cmark_node_append_child(this->tree_ptr.get(), empty_row);
            this->end_row_num += 1;
        }
    }
}

bool  MdTree::error()
{
    return this->tree_ptr == nullptr;
}

int MdTree::start_line()
{
    return cmark_node_get_start_line(this->get_root_ptr());
}

int MdTree::end_line()
{
    return this->end_row_num;
}
