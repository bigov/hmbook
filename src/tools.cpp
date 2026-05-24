#include "tools.h"

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
