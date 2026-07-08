#ifndef MD_PARSER_H
#define MD_PARSER_H

#define CMARK_NO_SHORT_NAMES

extern "C" {
#include <cmark-gfm.h>
#include <cmark-gfm-core-extensions.h>
#include <table.h>
}

void debug_node(cmark_node* node);
static const char* cmark_type_to_const_name(cmark_node_type t);


#endif // MD_PARSER_H