/**
  Markdown парсер на основе библиотеки md4c строит дерево md_node из событийного потока колбэков md4c.
  
  md4c - это SAX-подобный парсера (событийный, без построения дерева). Строить древовидную структуру
  md_node нужно через колбэки enter_block, leave_block, enter_span, leave_span, text.
  
Ключевые решения:

	Древовидная структура md_node содержит std::vector<std::unique_ptr<md_node>> children,
		что позволяет строить полное дерево документа без копирования.
	Построение через стек "md_build_ctx" стек указателей отслеживает текущий открытый контейнер.
		При enter_block/enter_span создаётся дочерний узел и он помещается на вершину стека.
		При leave_* — стек сокращается.
	Флаги MD_DIALECT_GITHUB — включены расширения (таблицы, strikethrough, task lists), 
		аналогично GitHub Flavored Markdown.
	node_debug — рекурсивный обход дерева с отступами и выводом атрибутов для отладки в консоли.
 */

#include "md_parser.h"
#include <stack>
#include <sstream>

// Вспомогательная функция: извлечь строку из MD_ATTRIBUTE
static std::string attr_to_string(const MD_ATTRIBUTE* attr)
{
	if (!attr || !attr->text || attr->size == 0) return {};
	return std::string(attr->text, attr->size);
}

// Преобразование MD_ALIGN (md4c) в md_align
static md_align convert_align(MD_ALIGN a)
{
	switch (a) {
	case MD_ALIGN_LEFT:   return MD_ALIGN_LEFT;
	case MD_ALIGN_CENTER: return MD_ALIGN_CENTER;
	case MD_ALIGN_RIGHT:  return MD_ALIGN_RIGHT;
	default:              return MD_ALIGN_NONE;
	}
}

// Преобразование MD_BLOCKTYPE в md_node_type
static md_node_type block_to_node_type(MD_BLOCKTYPE type)
{
	switch (type) {
	case MD_BLOCK_DOC:   return MD_NODE_DOC;
	case MD_BLOCK_QUOTE: return MD_NODE_QUOTE;
	case MD_BLOCK_UL:    return MD_NODE_UL;
	case MD_BLOCK_OL:    return MD_NODE_OL;
	case MD_BLOCK_LI:    return MD_NODE_LI;
	case MD_BLOCK_HR:    return MD_NODE_HR;
	case MD_BLOCK_H:     return MD_NODE_H;
	case MD_BLOCK_CODE:  return MD_NODE_CODE_BLOCK;
	case MD_BLOCK_HTML:  return MD_NODE_HTML_BLOCK;
	case MD_BLOCK_P:     return MD_NODE_P;
	case MD_BLOCK_TABLE: return MD_NODE_TABLE;
	case MD_BLOCK_THEAD: return MD_NODE_THEAD;
	case MD_BLOCK_TBODY: return MD_NODE_TBODY;
	case MD_BLOCK_TR:    return MD_NODE_TR;
	case MD_BLOCK_TH:    return MD_NODE_TH;
	case MD_BLOCK_TD:    return MD_NODE_TD;
	default:             return MD_NODE_NONE;
	}
}

// Преобразование MD_SPANTYPE в md_node_type
static md_node_type span_to_node_type(MD_SPANTYPE type)
{
	switch (type) {
	case MD_SPAN_EM:     return MD_NODE_EM;
	case MD_SPAN_STRONG: return MD_NODE_STRONG;
	case MD_SPAN_A:      return MD_NODE_A;
	case MD_SPAN_IMG:    return MD_NODE_IMG;
	case MD_SPAN_CODE:   return MD_NODE_CODE_SPAN;
	case MD_SPAN_DEL:    return MD_NODE_DEL;
	case MD_SPAN_U:      return MD_NODE_U;
	default:             return MD_NODE_NONE;
	}
}

// Преобразование MD_TEXTTYPE в md_node_type
static md_node_type text_to_node_type(MD_TEXTTYPE type)
{
	switch (type) {
	case MD_TEXT_NORMAL:   return MD_NODE_TEXT;
	case MD_TEXT_NULLCHAR: return MD_NODE_TEXT;
	case MD_TEXT_BR:       return MD_NODE_BR;
	case MD_TEXT_SOFTBR:   return MD_NODE_SOFTBR;
	case MD_TEXT_ENTITY:   return MD_NODE_TEXT_ENTITY;
	case MD_TEXT_CODE:     return MD_NODE_TEXT_CODE;
	case MD_TEXT_HTML:     return MD_NODE_TEXT_HTML;
	default:               return MD_NODE_TEXT;
	}
}

// Контекст построения дерева: стек текущих открытых узлов
struct md_build_ctx {
	md_node* root = nullptr;       // указатель на корневой узел (владеет вызывающая сторона)
	std::stack<md_node*> stack;    // стек указателей на текущий контейнер
};

// --- Колбэки md4c ---

static int cb_enter_block(MD_BLOCKTYPE type, void* detail, void* userdata)
{
	auto* ctx = static_cast<md_build_ctx*>(userdata);

	auto node = std::make_unique<md_node>();
	node->type = block_to_node_type(type);
	node->kind = MD_NODE_KIND_BLOCK;

	// Извлечение атрибутов из detail
	switch (type) {
	case MD_BLOCK_H: {
		auto* d = static_cast<MD_BLOCK_H_DETAIL*>(detail);
		if (d) node->heading_level = d->level;
		break;
	}
	case MD_BLOCK_OL: {
		auto* d = static_cast<MD_BLOCK_OL_DETAIL*>(detail);
		if (d) { node->ol_start = d->start; node->is_tight = d->is_tight; }
		break;
	}
	case MD_BLOCK_UL: {
		auto* d = static_cast<MD_BLOCK_UL_DETAIL*>(detail);
		if (d) node->is_tight = d->is_tight;
		break;
	}
	case MD_BLOCK_LI: {
		auto* d = static_cast<MD_BLOCK_LI_DETAIL*>(detail);
		if (d) { node->is_task = d->is_task; node->task_mark = d->task_mark; }
		break;
	}
	case MD_BLOCK_CODE: {
		auto* d = static_cast<MD_BLOCK_CODE_DETAIL*>(detail);
		if (d) node->fence_info = attr_to_string(&d->info);
		break;
	}
	case MD_BLOCK_TH: {
		auto* d = static_cast<MD_BLOCK_TD_DETAIL*>(detail);
		if (d) node->align = convert_align(d->align);
		break;
	}
	case MD_BLOCK_TD: {
		auto* d = static_cast<MD_BLOCK_TD_DETAIL*>(detail);
		if (d) node->align = convert_align(d->align);
		break;
	}
	default:
		break;
	}

	// Добавить узел как дочерний к текущему контейнеру
	md_node* parent = ctx->stack.top();
	md_node* raw = node.get();
	parent->children.push_back(std::move(node));
	// Новый узел становится текущим контейнером
	ctx->stack.push(raw);

	return 0;
}

static int cb_leave_block(MD_BLOCKTYPE type, void* detail, void* userdata)
{
	(void)type; (void)detail;
	auto* ctx = static_cast<md_build_ctx*>(userdata);
	if (ctx->stack.size() > 1) ctx->stack.pop();
	return 0;
}

static int cb_enter_span(MD_SPANTYPE type, void* detail, void* userdata)
{
	auto* ctx = static_cast<md_build_ctx*>(userdata);

	auto node = std::make_unique<md_node>();
	node->type = span_to_node_type(type);
	node->kind = MD_NODE_KIND_SPAN;

	// Извлечение атрибутов спана
	switch (type) {
	case MD_SPAN_A: {
		auto* d = static_cast<MD_SPAN_A_DETAIL*>(detail);
		if (d) {
			node->href = attr_to_string(&d->href);
			node->title = attr_to_string(&d->title);
		}
		break;
	}
	case MD_SPAN_IMG: {
		auto* d = static_cast<MD_SPAN_IMG_DETAIL*>(detail);
		if (d) {
			node->src = attr_to_string(&d->src);
			node->title = attr_to_string(&d->title);
		}
		break;
	}
	default:
		break;
	}

	md_node* parent = ctx->stack.top();
	md_node* raw = node.get();
	parent->children.push_back(std::move(node));
	ctx->stack.push(raw);

	return 0;
}

static int cb_leave_span(MD_SPANTYPE type, void* detail, void* userdata)
{
	(void)type; (void)detail;
	auto* ctx = static_cast<md_build_ctx*>(userdata);
	if (ctx->stack.size() > 1) ctx->stack.pop();
	return 0;
}

static int cb_text(MD_TEXTTYPE type, const MD_CHAR* text, MD_SIZE size, void* userdata)
{
	auto* ctx = static_cast<md_build_ctx*>(userdata);

	auto node = std::make_unique<md_node>();
	node->type = text_to_node_type(type);
	node->kind = MD_NODE_KIND_TEXT;

	if (type == MD_TEXT_NULLCHAR) {
		node->text = "\xEF\xBF\xBD"; // U+FFFD
	} else if (text && size > 0) {
		node->text.assign(text, size);
	}

	md_node* parent = ctx->stack.top();
	parent->children.push_back(std::move(node));

	return 0;
}

// --- Основная функция парсинга ---

int md_parce(const std::string &text, md_node &out)
{
	out = md_node{};
	out.type = MD_NODE_DOC;
	out.kind = MD_NODE_KIND_BLOCK;

	md_build_ctx ctx;
	ctx.root = &out;
	ctx.stack.push(&out);

	MD_PARSER parser = {};
	parser.abi_version = 0;
	parser.flags = MD_DIALECT_GITHUB;
	parser.enter_block = cb_enter_block;
	parser.leave_block = cb_leave_block;
	parser.enter_span  = cb_enter_span;
	parser.leave_span  = cb_leave_span;
	parser.text        = cb_text;
	parser.debug_log   = nullptr;
	parser.syntax      = nullptr;

	int rc = md_parse(text.c_str(), static_cast<MD_SIZE>(text.size()), &parser, &ctx);
	return rc;
}

// --- Отладочный вывод дерева ---

static const char* node_type_name(md_node_type type)
{
	switch (type) {
	case MD_NODE_NONE:        return "NONE";
	case MD_NODE_DOC:         return "DOC";
	case MD_NODE_QUOTE:       return "QUOTE";
	case MD_NODE_UL:          return "UL";
	case MD_NODE_OL:          return "OL";
	case MD_NODE_LI:          return "LI";
	case MD_NODE_HR:          return "HR";
	case MD_NODE_H:           return "H";
	case MD_NODE_CODE_BLOCK:  return "CODE_BLOCK";
	case MD_NODE_HTML_BLOCK:  return "HTML_BLOCK";
	case MD_NODE_P:           return "P";
	case MD_NODE_TABLE:       return "TABLE";
	case MD_NODE_THEAD:       return "THEAD";
	case MD_NODE_TBODY:       return "TBODY";
	case MD_NODE_TR:          return "TR";
	case MD_NODE_TH:          return "TH";
	case MD_NODE_TD:          return "TD";
	case MD_NODE_EM:          return "EM";
	case MD_NODE_STRONG:      return "STRONG";
	case MD_NODE_A:           return "A";
	case MD_NODE_IMG:         return "IMG";
	case MD_NODE_CODE_SPAN:   return "CODE_SPAN";
	case MD_NODE_DEL:         return "DEL";
	case MD_NODE_U:           return "U";
	case MD_NODE_TEXT:        return "TEXT";
	case MD_NODE_SOFTBR:      return "SOFTBR";
	case MD_NODE_BR:          return "BR";
	case MD_NODE_TEXT_CODE:   return "TEXT_CODE";
	case MD_NODE_TEXT_HTML:   return "TEXT_HTML";
	case MD_NODE_TEXT_ENTITY: return "TEXT_ENTITY";
	default:                   return "UNKNOWN";
	}
}

std::string node_debug(const md_node &node, int depth)
{
	std::ostringstream ss;
	std::string indent(depth * 2, ' ');

	ss << indent << node_type_name(node.type);

	// Дополнительные атрибуты
	if (node.type == MD_NODE_H)
		ss << " level=" << node.heading_level;
	if (node.type == MD_NODE_OL)
		ss << " start=" << node.ol_start << " tight=" << node.is_tight;
	if (node.type == MD_NODE_UL)
		ss << " tight=" << node.is_tight;
	if (node.type == MD_NODE_LI && node.is_task)
		ss << " task mark='" << node.task_mark << "'";
	if (node.type == MD_NODE_CODE_BLOCK && !node.fence_info.empty())
		ss << " info=\"" << node.fence_info << "\"";
	if (node.type == MD_NODE_A)
		ss << " href=\"" << node.href << "\"";
	if (node.type == MD_NODE_IMG)
		ss << " src=\"" << node.src << "\"";
	if ((node.type == MD_NODE_TD || node.type == MD_NODE_TH) && node.align != MD_ALIGN_NONE) {
		static const char* align_names[] = { "none", "left", "center", "right" };
		ss << " align=" << align_names[node.align];
	}

	// Текстовое содержимое
	if (!node.text.empty()) {
		// Экранирование переносов строк для удобства чтения
		std::string escaped = node.text;
		for (size_t i = 0; i < escaped.size(); i++) {
			if (escaped[i] == '\n') { escaped.replace(i, 1, "\\n"); i++; }
			else if (escaped[i] == '\r') { escaped.replace(i, 1, "\\r"); i++; }
		}
		ss << " \"" << escaped << "\"";
	}

	ss << "\n";

	for (const auto& child : node.children)
		ss << node_debug(*child, depth + 1);

	return ss.str();
}