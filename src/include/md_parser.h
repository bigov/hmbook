#ifndef MD_PARSER_H
#define MD_PARSER_H

#include <string>
#include <vector>
#include <memory>
#include "md4c.h"


// Категория узла
enum md_node_kind {
	MD_NODE_KIND_BLOCK,
	MD_NODE_KIND_SPAN,
	MD_NODE_KIND_TEXT,
};

// Тип узла дерева — объединяет блоки, спаны и текст md4c
enum md_node_type {
	MD_NODE_NONE = 0,
	// Блоки
	MD_NODE_DOC,
	MD_NODE_QUOTE,
	MD_NODE_UL,
	MD_NODE_OL,
	MD_NODE_LI,
	MD_NODE_HR,
	MD_NODE_H,
	MD_NODE_CODE_BLOCK,
	MD_NODE_HTML_BLOCK,
	MD_NODE_P,
	MD_NODE_TABLE,
	MD_NODE_THEAD,
	MD_NODE_TBODY,
	MD_NODE_TR,
	MD_NODE_TH,
	MD_NODE_TD,
	// Спаны
	MD_NODE_EM,
	MD_NODE_STRONG,
	MD_NODE_A,
	MD_NODE_IMG,
	MD_NODE_CODE_SPAN,
	MD_NODE_DEL,
	MD_NODE_U,
	// Текст
	MD_NODE_TEXT,
	MD_NODE_SOFTBR,
	MD_NODE_BR,
	MD_NODE_TEXT_CODE,
	MD_NODE_TEXT_HTML,
	MD_NODE_TEXT_ENTITY,
};

// Выравнивание ячейки таблицы
enum md_align {
	MD_ALIGN_NONE = 0,
	MD_ALIGN_LEFT,
	MD_ALIGN_CENTER,
	MD_ALIGN_RIGHT,
};

typedef struct md_node {
	md_node_type type = MD_NODE_NONE;
	md_node_kind kind = MD_NODE_KIND_BLOCK;

	// Текстовое содержимое (для текстовых узлов)
	std::string text;

	// Атрибуты блоков/спанов
	unsigned heading_level = 0;      // MD_BLOCK_H: уровень заголовка 1-6
	unsigned ol_start = 0;           // MD_BLOCK_OL: начальный номер
	int is_tight = 0;               // MD_BLOCK_UL/OL: плотный список
	int is_task = 0;                // MD_BLOCK_LI: элемент является задачей (чекбокс)
	char task_mark = 0;             // MD_BLOCK_LI: символ отметки ('x', 'X', ' ', 0)
	md_align align = MD_ALIGN_NONE; // MD_BLOCK_TD/TH: выравнивание ячейки
	std::string fence_info;          // MD_BLOCK_CODE: info-строка (язык)
	std::string href;                // MD_SPAN_A: ссылка
	std::string title;               // MD_SPAN_A / MD_SPAN_IMG: title
	std::string src;                 // MD_SPAN_IMG: src

	// Дочерние узлы (unique_ptr для снижения стоимости перемещений)
	std::vector<std::unique_ptr<md_node>> children;
} md_node;


// Парсинг markdown-текста в дерево узлов.
// Возвращает 0 при успехе, ненулевое значение при ошибке парсинга.
int md_parce(const std::string &text, md_node &out);

std::string node_debug(const md_node &node, int depth = 0);

#endif //MD_PARSER_H