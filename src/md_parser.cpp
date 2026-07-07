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
	node->type = type;
	node->kind = BLOCK;

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
		break;
	}
	case MD_BLOCK_TD: {
		auto* d = static_cast<MD_BLOCK_TD_DETAIL*>(detail);
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
	node->type = type;
	node->kind = SPAN;

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
	node->type = type;
	node->kind = TEXT;

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
	out.type = MD_BLOCK_DOC;
	out.kind = BLOCK;

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

// Имя типа узла для отладочного вывода
static const char* node_type_name(const md_node &node)
{
	switch ((node.kind << 8) | node.type) {
	// Блоки
	case (BLOCK << 8) | MD_BLOCK_DOC:   return "DOC";
	case (BLOCK << 8) | MD_BLOCK_QUOTE: return "QUOTE";
	case (BLOCK << 8) | MD_BLOCK_UL:    return "UL";
	case (BLOCK << 8) | MD_BLOCK_OL:    return "OL";
	case (BLOCK << 8) | MD_BLOCK_LI:    return "LI";
	case (BLOCK << 8) | MD_BLOCK_HR:    return "HR";
	case (BLOCK << 8) | MD_BLOCK_H:     return "H";
	case (BLOCK << 8) | MD_BLOCK_CODE:  return "CODE_BLOCK";
	case (BLOCK << 8) | MD_BLOCK_HTML:  return "HTML_BLOCK";
	case (BLOCK << 8) | MD_BLOCK_P:     return "P";
	case (BLOCK << 8) | MD_BLOCK_TABLE: return "TABLE";
	case (BLOCK << 8) | MD_BLOCK_THEAD: return "THEAD";
	case (BLOCK << 8) | MD_BLOCK_TBODY: return "TBODY";
	case (BLOCK << 8) | MD_BLOCK_TR:    return "TR";
	case (BLOCK << 8) | MD_BLOCK_TH:    return "TH";
	case (BLOCK << 8) | MD_BLOCK_TD:    return "TD";
	// Спаны
	case (SPAN << 8) | MD_SPAN_EM:      return "EM";
	case (SPAN << 8) | MD_SPAN_STRONG:  return "STRONG";
	case (SPAN << 8) | MD_SPAN_A:       return "A";
	case (SPAN << 8) | MD_SPAN_IMG:     return "IMG";
	case (SPAN << 8) | MD_SPAN_CODE:    return "CODE_SPAN";
	case (SPAN << 8) | MD_SPAN_DEL:     return "DEL";
	case (SPAN << 8) | MD_SPAN_U:       return "U";
	// Текст
	case (TEXT << 8) | MD_TEXT_NORMAL:   return "TEXT";
	case (TEXT << 8) | MD_TEXT_NULLCHAR: return "TEXT";
	case (TEXT << 8) | MD_TEXT_BR:       return "BR";
	case (TEXT << 8) | MD_TEXT_SOFTBR:   return "SOFTBR";
	case (TEXT << 8) | MD_TEXT_ENTITY:   return "TEXT_ENTITY";
	case (TEXT << 8) | MD_TEXT_CODE:     return "TEXT_CODE";
	case (TEXT << 8) | MD_TEXT_HTML:     return "TEXT_HTML";
	default:                                          return "UNKNOWN";
	}
}

std::string node_debug(const md_node &node, int depth)
{
	std::ostringstream ss;
	std::string indent(depth * 2, ' ');

	ss << indent << node_type_name(node);

	// Дополнительные атрибуты
	if (node.type == MD_BLOCK_H)
		ss << " level=" << node.heading_level;
	if (node.type == MD_BLOCK_OL)
		ss << " start=" << node.ol_start << " tight=" << node.is_tight;
	if (node.type == MD_BLOCK_UL)
		ss << " tight=" << node.is_tight;
	if (node.type == MD_BLOCK_LI && node.is_task)
		ss << " task mark='" << node.task_mark << "'";
	if (node.type == MD_BLOCK_CODE && !node.fence_info.empty())
		ss << " info=\"" << node.fence_info << "\"";
	if (node.type == MD_SPAN_A)
		ss << " href=\"" << node.href << "\"";
	if (node.type == MD_SPAN_IMG)
		ss << " src=\"" << node.src << "\"";
	if ((node.type == MD_BLOCK_TD || node.type == MD_BLOCK_TH) && node.align != MD_ALIGN_DEFAULT) {
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