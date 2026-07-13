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

    bool error();
    int start_line();
    int end_line();
    cmark_node* get_root_ptr() const { return tree_ptr.get(); }

private:
    std::unique_ptr<cmark_node, CmarkNodeDeleter> tree_ptr;
    int end_row_num = 0;
};



#endif // MD_PARSER_H