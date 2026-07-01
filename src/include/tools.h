#ifndef HMB_TOOLS_H
#define HMB_TOOLS_H

#include <memory>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <cstring>

//#include <bits/stdc++.h>

#include <wx/menu.h>
#include <wx/richtext/richtextctrl.h>
#include <wx/richtext/richtextxml.h>
#include <wx/wfstream.h>
#include <wx/string.h>
#include <wx/sstream.h>
#include <wx/colordlg.h>
#include <wx/fontdlg.h>
#include <wx/textdlg.h>
#include <wx/tokenzr.h>
#include <wx/log.h>

extern "C" {
#include <cmark.h>
}

// Идентификатор кнопки переключения переноса слов на вкладке "Text".
enum {
    hmbID_LINE_WRAPPING = wxID_HIGHEST + 1,
    hmbID_SHOW_BUFFER_XML
};

extern wxString HMB_FNAME;
extern wxString HMB_DNAME;
extern std::string HMB_SRC_DATA;

extern wxColor HMB_COLOR_BASE_FG;
extern wxColor HMB_COLOR_BASE_BG;
extern wxFont HMB_FONT_BASE;
extern wxFont HMB_FONT_MONO;

static const wxString RICH_BUFFER_EXT = "wxrt";
static const wxString TEXT_BUFFER_EXT = "txt";
static const wxString MARK_BUFFER_EXT = "md";

bool isFileExist(const wxString filePath);
void file_read(const wxString filePath, std::string &target_string);
void file_write(const std::string &plain_text, const wxString &file_path);
wxString hmb_decode_xml(const wxString& text);
void wx_to_utf8(const wxString& src_data, std::string& dst_string);
std::string replace_placeholder(std::string tpl,
                          const std::string& placeholder,
                          const std::string& content);

int digits(long long n); // Количество разрядов в числе
void debug_node(cmark_node* node);
static const char* cmark_type_to_const_name(cmark_node_type t);

#endif // HMB_TOOLS_H
