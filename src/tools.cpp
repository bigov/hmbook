#include "hmb/tools.h"

// Глобальная переменная для хранения исходного текста, загруженного из файла.
std::string HMB_SRC_DATA = ""; 

// Проверка существования файла по указанному пути. Вызывается из panel_tree при выборе файла в дереве.
bool isFileExist(const wxString filePath)
{
    if (wxFileExists(filePath)) return true;
    wxLogError(_("Not found file '%s'."), filePath.wc_str());
    return false;
}

// Чтение исходного текста из файла. Вызывается из panel_tree при выборе файла в дереве.
void file_read(const wxString filePath, std::string &target_string)
{
    target_string.clear();
    const wchar_t* f = filePath.wc_str();
    if (std::ifstream reader{f}; reader)
   {
        std::string line;
        while (std::getline(reader, line)) target_string.append(line + "\n");
    } else {
        wxLogError(_("Cannot read file '%s'."), filePath.wc_str());
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


// Замена всех вхождений подстроки placeholder в строке tpl на строку content.
// Вызывается при сохранении текста из редактора.
std::string to_utf8(const wxString& value)
{
    const wxScopedCharBuffer utf8 = value.ToUTF8();
    return utf8.data() ? std::string(utf8.data()) : std::string();
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

