#include "main_panel.h"

#include <fstream>
#include <sstream>
#include <cstdlib>

#include "txt_rich.h"
#include "wx/button.h"
#include "wx/simplebook.h"
#include "wx/sizer.h"
#include "wx/textctrl.h"

namespace
{
    wxString decode_numeric_xml_entities(const wxString& text)
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
}

MainPanel::MainPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME)
{
    SetBackgroundColour(wxColour("#ffffff"));

    m_book = new wxSimplebook(
        this,
        wxID_ANY,
        wxDefaultPosition,
        wxDefaultSize,
        wxBORDER_NONE
    );

    m_txtRich = new TxtRich(m_book);
    m_source = new wxTextCtrl(
        m_book,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxTE_MULTILINE | wxBORDER_NONE | wxTE_NO_VSCROLL
    );

    m_buffer = new wxTextCtrl(
        m_book,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxTE_MULTILINE | wxBORDER_NONE | wxTE_NO_VSCROLL
    );

    m_book->AddPage(m_txtRich, "txt_rich", true);
    m_book->AddPage(m_source, "source", false);
    m_book->AddPage(m_buffer, "buffer", false);

    wxPanel* tabsPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition,
                                     wxDefaultSize, wxBORDER_NONE);
    //tabsPanel->SetBackgroundColour(GetBackgroundColour());
    tabsPanel->SetBackgroundColour("#EFEFEF");
    

    wxButton* tabBtnRich = new wxButton(tabsPanel, wxID_ANY, "txt_rich", wxDefaultPosition,
                                        wxDefaultSize, 0);
    wxButton* tabBtnSource = new wxButton(tabsPanel, wxID_ANY, "source", wxDefaultPosition,
                                        wxDefaultSize, 0);
    wxButton* tabBtnBuffer = new wxButton(tabsPanel, wxID_ANY, "buffer", wxDefaultPosition,
                                        wxDefaultSize, 0);

    tabBtnRich->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { m_book->SetSelection(0); });
    
    tabBtnSource->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        const wxString filePath = m_txtRich->current_filePath;
        if (!filePath.IsEmpty())
        {
            std::ifstream input(filePath.wc_str(), std::ios::binary);
            if (input)
            {
                std::ostringstream buffer;
                buffer << input.rdbuf();
                m_source->ChangeValue(wxString::FromUTF8(buffer.str()));
            }
            else
            {
                m_source->Clear();
            }
        }
        else
        {
            m_source->Clear();
        }
        m_book->SetSelection(1);
    });
    
    tabBtnBuffer->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        const wxString xml_text = m_txtRich->export_xml_text();
        m_buffer->ChangeValue(decode_numeric_xml_entities(xml_text));
        m_book->SetSelection(2);
    });

    wxBoxSizer* tabsSizer = new wxBoxSizer(wxHORIZONTAL);
    tabsSizer->AddStretchSpacer(1);
    int buttonSpacing = 2; // Расстояние между кнопками
    int rightMargin = 16;  // Отступ справа от последней кнопки
   
    tabsSizer->Add(tabBtnRich, 0, wxRIGHT, buttonSpacing);
    tabsSizer->Add(tabBtnSource, 0, wxRIGHT, buttonSpacing);
    tabsSizer->Add(tabBtnBuffer, 0, wxRIGHT, rightMargin);
   
    tabsPanel->SetSizer(tabsSizer);

    wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);
    rootSizer->Add(m_book, 1, wxEXPAND | wxALL, 0);
    rootSizer->Add(tabsPanel, 0, wxEXPAND | wxALL, 0);
    SetSizer(rootSizer);
}

TxtRich* MainPanel::get_txt_rich() const
{
    return m_txtRich;
}
