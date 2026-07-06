#include "tools.h"

// Глобальная переменная для хранения исходного текста, загруженного из файла.
std::string HMB_SRC_DATA = ""; 

// Количество разрядов в числе
int digits(long long n) {
    if (n < 10) return 1;
    n = llabs(n);
    return (int)floor(log10((long double)n)) + 1;
}

bool isFileExist(const wxString filePath)
{
    if (wxFileExists(filePath)) return true;
    wxLogError(_("Not found file '%s'."), filePath.wc_str());
    return false;
}

void file_read(const wxString filePath, std::string &target_string)
{
    target_string.clear();
    const wchar_t* f = filePath.wc_str();
    if (std::ifstream reader{f, std::ios::binary}; reader)
    {
        target_string.assign(
            std::istreambuf_iterator<char>(reader),
            std::istreambuf_iterator<char>()
        );
    } else {
        wxLogError(_("Cannot read file '%s'."), filePath.wc_str());
    }
}


void file_write(const std::string &plain_text, const wxString &file_path)
{
    wxFileOutputStream output_stream(file_path);
    output_stream.Write(plain_text.data(), plain_text.length());

    if (output_stream.GetLastError() != wxSTREAM_NO_ERROR)
    {
        wxLogError(_("Error while writing to file '%s'."), file_path.wc_str());
        return;
    }
}

// Перекодирование символов вида &#xNNNN; или &#DDDD; в соответствующие Unicode символы.
// Вызывается при загрузке текста в редактор и при сохранении текста из редактора.
wxString hmb_decode_xml(const wxString& text)
{
    wxString out;
    out.reserve(text.length());

    size_t i = 0;
    while (i < text.length())
    {
        if (text[i] == '&' && (i + 2) < text.length() && text[i + 1] == '#')
        {
            const bool is_hex = (text[i + 2] == 'x' || text[i + 2] == 'X');
            size_t num_start = i + (is_hex ? 3 : 2);
            size_t j = num_start;

            while (j < text.length())
            {
                const wxUniChar c = text[j];
                const bool ok = is_hex
                    ? ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
                    : (c >= '0' && c <= '9');
                if (!ok) break;
                ++j;
            }

            if (j > num_start && j < text.length() && text[j] == ';')
            {
                const wxString number = text.Mid(num_start, j - num_start);
                const std::string number_utf8 = number.ToStdString();
                char* end_ptr = nullptr;
                const long codepoint = std::strtol(number_utf8.c_str(), &end_ptr, is_hex ? 16 : 10);
                if (end_ptr != number_utf8.c_str() && *end_ptr == '\0' && codepoint > 0 && codepoint <= 0x10FFFF)
                {
                    out += wxUniChar(static_cast<int>(codepoint));
                    i = j + 1;
                    continue;
                }
            }
        }

        out += text[i];
        ++i;
    }

    return out;
}

// Перекодирование wxString в UTF-8 std::string.
void wx_to_utf8(const wxString& src_data, std::string& dst_string)
{
    dst_string.clear();
    const wxScopedCharBuffer utf8 = src_data.ToUTF8();
    dst_string = utf8.data() ? std::string(utf8.data()) : std::string();
}

// Замена всех вхождений подстроки placeholder в строке tpl на строку content.
// Вызывается при сохранении текста из редактора.
std::string replace_placeholder(std::string tpl,
                          const std::string& placeholder,
                          const std::string& content)
{
    size_t pos = 0;
    while ((pos = tpl.find(placeholder, pos)) != std::string::npos)
    {
        tpl.replace(pos, placeholder.size(), content);
        pos += content.size();
    }
    return tpl;
}

// ---------------------------------------------
// --- диапазон строк ноды (для печати отладки) ---
static const char* cmark_type_to_const_name(cmark_node_type t)
{
    switch (t)
    {
    case CMARK_NODE_NONE: return "NONE";
    case CMARK_NODE_DOCUMENT: return "DOCUMENT";
    case CMARK_NODE_BLOCK_QUOTE: return "BLOCK_QUOTE";
    case CMARK_NODE_LIST: return "LIST";
    case CMARK_NODE_ITEM: return "ITEM";
    case CMARK_NODE_CODE_BLOCK: return "CODE_BLOCK";
    case CMARK_NODE_HTML_BLOCK: return "HTML_BLOCK";
    case CMARK_NODE_CUSTOM_BLOCK: return "CUSTOM_BLOCK";
    case CMARK_NODE_PARAGRAPH: return "PARAGRAPH";
    case CMARK_NODE_HEADING: return "HEADING";
    case CMARK_NODE_THEMATIC_BREAK: return "THEMATIC_BREAK";
    case CMARK_NODE_TEXT: return "TEXT";
    case CMARK_NODE_SOFTBREAK: return "SOFTBREAK";
    case CMARK_NODE_LINEBREAK: return "LINEBREAK";
    case CMARK_NODE_CODE: return "CODE";
    case CMARK_NODE_HTML_INLINE: return "HTML_INLINE";
    case CMARK_NODE_CUSTOM_INLINE: return "CUSTOM_INLINE";
    case CMARK_NODE_EMPH: return "EMPH";
    case CMARK_NODE_STRONG: return "STRONG";
    case CMARK_NODE_LINK: return "LINK";
    case CMARK_NODE_IMAGE: return "IMAGE";
    default: return "UNKNOWN";
    }
}

void debug_node(cmark_node* node) {
    int start_line = 0;
    int end_line = 0;
    cmark_node_type t = CMARK_NODE_NONE;
    if (node) 
    { 
        start_line = cmark_node_get_start_line(node);
        end_line = cmark_node_get_end_line(node);
        t = cmark_node_get_type(node);
    }
    std::cerr << "[" << start_line << " - " << end_line << "] " << cmark_type_to_const_name(t) << "\n";
}

// Запуск консольной команды без создания видимого окна.
// Возвращает код возврата процесса, или -1 при ошибке запуска.
int run_cmd_hidden(const std::string& cmd, const std::string& workDir)
{
    STARTUPINFOA si = {};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi = {};

    std::string cmdLine = "cmd.exe /C " + cmd;

    const char* dir = workDir.empty() ? nullptr : workDir.c_str();

    BOOL ok = CreateProcessA(
        nullptr,
        cmdLine.data(),
        nullptr, nullptr,
        FALSE,
        CREATE_NO_WINDOW,
        nullptr,
        dir,
        &si, &pi
    );

    if (!ok) return -1;

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return static_cast<int>(exitCode);
}