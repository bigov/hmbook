// Markdown парсер
#include "md_parser.h"

enum md_node_type {
	MD_NODE_NONE;
}

typedef struct cmark_node {
	md_node_type;
	int line_start = 0;
	int line_end = 0;
} cmark_node;

class hmbParser {
public:

	hmbParser();
	
	cmark_node parse_document(std::string &string)
	{
		return cmark_node();
	};

private:
}