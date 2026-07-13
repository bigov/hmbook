#ifndef MD_PARSER_H
#define MD_PARSER_H

#include <memory>

#define CMARK_NO_SHORT_NAMES

extern "C" {
#include <cmark-gfm.h>
#include <cmark-gfm-core-extensions.h>
#include <table.h>
}

struct CmarkNodeDeleter {
    void operator()(cmark_node* node) const { if (node) cmark_node_free(node); }
};

void debug_node(cmark_node* node);
static const char* cmark_type_to_const_name(cmark_node_type t);

class MdTree
{
public: 
    MdTree(const std::string  &text);
    ~MdTree() = default;

    // Запрет копирования, чтобы избежать проблем с владением
    MdTree(const MdTree&) = delete;
    MdTree& operator=(const MdTree&) = delete;
    
    // Разрешение перемещения (опционально)
    MdTree(MdTree&&) = default;
    MdTree& operator=(MdTree&&) = default;

    cmark_node* get_raw_node() const { return node_ptr.get(); }
    //Если функция библиотеки возвращает новый cmark_node* (который вы хотите забрать себе), используем reset():
    // this->node_ptr.reset(cmark_parser_finish(parser));

    int start_line();
    int end_line();
    bool error();

private:
    //cmark_node* node_ptr = nullptr;
    std::unique_ptr<cmark_node, CmarkNodeDeleter> node_ptr;
    int end_row_num = 0;
};



#endif // MD_PARSER_H