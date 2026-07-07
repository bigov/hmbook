#ifndef MD_PARSER_H
#define MD_PARSER_H

#include <string>
#include <vector>
#include <memory>
#include "md4c.h"

// Категория узла. в md4c такого перечисления нет. Библиотека md4c — событийный
// парсер, у неё нет понятия «узел дерева». Категория узла определяется неявно — 
// через то, какой колбэк вызван (enter_block, enter_span, text). Единого enum,
// объединяющего block/span/text, в md4c не существует.
// node_kind введенно для того, чтобы различать пересекающиеся диапазоны
// MD_BLOCKTYPE, MD_SPANTYPE и MD_TEXTTYPE в едином поле type.
enum node_kind {
	BLOCK,
	SPAN,
	TEXT,
};

typedef struct md_node {
	int type = MD_BLOCK_DOC;                // MD_BLOCKTYPE | MD_SPANTYPE | MD_TEXTTYPE
	node_kind kind = BLOCK;

	// Текстовое содержимое (для текстовых узлов)
	std::string text;

	// Атрибуты блоков/спанов
	unsigned heading_level = 0;      // MD_BLOCK_H: уровень заголовка 1-6
	unsigned ol_start = 0;           // MD_BLOCK_OL: начальный номер
	int is_tight = 0;               // MD_BLOCK_UL/OL: плотный список
	int is_task = 0;                // MD_BLOCK_LI: элемент является задачей (чекбокс)
	char task_mark = 0;             // MD_BLOCK_LI: символ отметки ('x', 'X', ' ', 0)
	MD_ALIGN align = MD_ALIGN_DEFAULT; // MD_BLOCK_TD/TH: выравнивание ячейки
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