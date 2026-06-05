#ifndef HMB_TOOLS_H
#define HMB_TOOLS_H

#include <memory>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <cstring>

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
wxString hmb_decode_xml(const wxString& text);
std::string to_utf8(const wxString& value);
std::string replace_placeholder(std::string tpl,
                          const std::string& placeholder,
                          const std::string& content);

void debug_node(cmark_node* node);
static const char* cmark_type_to_const_name(cmark_node_type t);

#endif // HMB_TOOLS_H
