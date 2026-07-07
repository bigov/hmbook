# Hyper-Markdown Book

HM Book - это Графический интерфейс для работы с коллекцией файлов в формате Markdown, хранящихся в выделенной папке, с возможностью редактирования файлов в режиме WIYSWIYG. Главной отличительной чертой приложения HM Book от аналогичных является то, что оно позволяет работать с файлами коллекции, не искажая их формат, не внося никаких служебных измений или дополнений в файлы или в папку, в которой хранится весь набор. Для управления настройками приложение использует собственные файлы конфигурации, хранящиеся отдельно от коллекции.

MDBook написан на языке программирования C++. Для удобства работы использует библиотеки cmark и wxWidgets.

## Справочная документация

 - [DeepWiki commonmark/cmark](https://deepwiki.com/commonmark/cmark/1-overview)
 - Документации [wxRichTextCtrl](https://docs.wxwidgets.org/stable/overview_richtextctrl.html)

 
## cmark-gfm

- Репозиторий: https://github.com/github/cmark-gfm
- Это форк библиотеки cmark от GitHub, расширенный поддержкой GitHub Flavored Markdown (GFM).
- Текущее состояние: последний релиз — 0.29.0.gfm.13 (2024).
- Лицензия: BSD-2-Clause (как и оригинальный cmark).

### Возможности

**Всё из cmark:**

- Полное дерево AST (не событийный, а древовидный парсер).
- Каждый узел хранит start_line, start_column, end_line, end_column — точные позиции в исходном тексте.
- Итератор по дереву (cmark_iter) — обход без рекурсии.
- Встроенный рендер в HTML, XML, man, CommonMark, LaTeX.

**Расширения GFM (подключаются как плагины):**

- Таблицы (table) — полная поддержка GFM-таблиц с выравниванием столбцов.
- Strikethrough (strikethrough) — зачёркнутый текст ~~text~~.
- Autolink (autolink) — автоматические ссылки из URL/email.
- Tag filter (tagfilter) — фильтрация опасных HTML-тегов.
- Task lists (tasklist) — чекбоксы - [x].
- API (ключевые функции)

```c++
// Парсинг
cmark_parser *parser = cmark_parser_new(CMARK_OPT_DEFAULT);
cmark_parser_attach_syntax_extension(parser, cmark_find_syntax_extension("table"));
cmark_parser_attach_syntax_extension(parser, cmark_find_syntax_extension("strikethrough"));
cmark_parser_attach_syntax_extension(parser, cmark_find_syntax_extension("tasklist"));
cmark_parser_feed(parser, markdown_text, len);
cmark_node *doc = cmark_parser_finish(parser);

// Позиции в исходнике
int sl = cmark_node_get_start_line(node);
int sc = cmark_node_get_start_column(node);
int el = cmark_node_get_end_line(node);
int ec = cmark_node_get_end_column(node);

// Обход дерева
cmark_iter *iter = cmark_iter_new(doc);
cmark_event_type ev;
while ((ev = cmark_iter_next(iter)) != CMARK_EVENT_DONE) {
    cmark_node *node = cmark_iter_get_node(iter);
    // ev == CMARK_EVENT_ENTER или CMARK_EVENT_EXIT
}

// Типы узлов таблиц
CMARK_NODE_TABLE
CMARK_NODE_TABLE_ROW
CMARK_NODE_TABLE_CELL

// Освобождение
cmark_iter_free(iter);
cmark_node_free(doc);
cmark_parser_free(parser);
```

Подключение к проекту

```cmake
add_subdirectory(3rdparty/cmark-gfm)
target_link_libraries(${PROJECT_NAME} PRIVATE libcmark-gfm libcmark-gfm-extensions)
```