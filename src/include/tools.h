#ifndef HMB_TOOLS_H
#define HMB_TOOLS_H

#include <memory>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <cstring>
#include <windows.h>

#include "wx/colordlg.h"
#include "wx/config.h"
#include "wx/dir.h"
#include "wx/dirdlg.h"
#include "wx/filedlg.h"
#include "wx/filefn.h"
#include "wx/filename.h"
#include "wx/fontdlg.h"
#include "wx/frame.h"
#include "wx/icon.h"
#include "wx/image.h"
#include "wx/log.h"
#include "wx/menu.h"
#include "wx/panel.h"
#include "wx/richtext/richtextctrl.h"
#include "wx/richtext/richtextxml.h"
#include "wx/sizer.h"
#include "wx/splitter.h"
#include "wx/sstream.h"
#include "wx/stdpaths.h"
#include "wx/string.h"
#include "wx/textdlg.h"
#include "wx/tokenzr.h"
#include "wx/wfstream.h"

// Идентификатор кнопки переключения переноса слов на вкладке "Text".
enum {
    hmbID_LINE_WRAPPING = wxID_HIGHEST + 1,
    hmbID_SHOW_BUFFER_XML
};

enum
{
    RICHTEXT_LEFT_ALIGN = hmbID_SHOW_BUFFER_XML + 1, // было = 1100,
    RICHTEXT_RIGHT_ALIGN,
    RICHTEXT_CENTRE,
    RICHTEXT_JUSTIFY,
    RICHTEXT_CHANGE_FONT,
    RICHTEXT_CHANGE_TEXT_COLOUR,
    RICHTEXT_CHANGE_BACKGROUND_COLOUR,
    RICHTEXT_LEFT_INDENT,
    RICHTEXT_RIGHT_INDENT,
    RICHTEXT_TAB_STOPS
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
int run_cmd_hidden(const std::string& cmd, const std::string& workDir = "");

#endif // HMB_TOOLS_H
