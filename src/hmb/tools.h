#ifndef HMB_TOOLS_H
#define HMB_TOOLS_H

#include <memory>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <cstring>

#include "wx/menu.h"
#include "wx/richtext/richtextctrl.h"
#include "wx/richtext/richtextxml.h"
#include "wx/wfstream.h"
#include "wx/string.h"
#include "wx/sstream.h"
#include "wx/colordlg.h"
#include "wx/fontdlg.h"
#include "wx/textdlg.h"
#include "wx/tokenzr.h"
#include "wx/log.h"

extern wxString HMB_FNAME;
extern wxString HMB_DNAME;
extern std::string HMB_SRC_DATA;

static const wxString RICH_BUFFER_EXT = "wxrt";
static const wxString TEXT_BUFFER_EXT = "txt";
static const wxString MARK_BUFFER_EXT = "md";

bool isFileExist(const wxString filePath);
void load_src_data(const wxString filePath);
wxString hmb_decode_xml(const wxString& text);

#endif // HMB_TOOLS_H
