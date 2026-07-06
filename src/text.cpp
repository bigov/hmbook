#include "text.h"

hmbText::hmbText(wxWindow* parent)
    : wxStyledTextCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 
        wxBORDER_NONE | wxVSCROLL )
{
    // Базовый стиль текста для всего буфера STC.
    this->StyleSetFont(wxSTC_STYLE_DEFAULT, HMB_FONT_MONO);
    this->StyleSetForeground(wxSTC_STYLE_DEFAULT, HMB_COLOR_BASE_FG);
    this->StyleSetBackground(wxSTC_STYLE_DEFAULT, HMB_COLOR_BASE_BG);
    this->StyleClearAll();

    // Дублируем цвета на уровне окна, чтобы не было визуальных артефактов фона.
    this->SetBackgroundColour(HMB_COLOR_BASE_BG);
    this->SetForegroundColour(HMB_COLOR_BASE_FG);

    this->SetMarginLeft(4);
    this->SetMarginRight(4);

    // Включить отображение нумерации строк в отдельном поле слева.
    this->SetMarginType(0, wxSTC_MARGIN_NUMBER);
    this->SetMarginWidth(0, this->TextWidth(wxSTC_STYLE_LINENUMBER, "_999"));

    // Отключить дополнительные служебные отступы для сворачивания и т.п.
    this->SetMarginWidth(1, 0);
    this->SetMarginWidth(2, 0);

    this->set_lexer(wxSTC_LEX_MARKDOWN);
    this->on_edit = false;
    this->Bind(wxEVT_STC_CHANGE, &hmbText::on_text_change, this);
}

// Переключить лексер подсветки и применить соответствующее оформление.
void hmbText::set_lexer(int lexer)
{
    this->SetLexer(lexer);

    switch(lexer)
    {
        case wxSTC_LEX_MARKDOWN:
            // Минимальная подсветка markdown
            this->StyleSetForeground(wxSTC_MARKDOWN_HEADER1, wxColour("#000000"));
            this->StyleSetForeground(wxSTC_MARKDOWN_HEADER2, wxColour("#000000"));
            this->SetProperty("lexer.markdown.header.eolfill", "1");

            // Заголовки markdown выделяем жирным шрифтом.
            this->StyleSetBold(wxSTC_MARKDOWN_HEADER1, true);
            this->StyleSetBold(wxSTC_MARKDOWN_HEADER2, true);
            this->StyleSetBold(wxSTC_MARKDOWN_HEADER3, true);
            this->StyleSetBold(wxSTC_MARKDOWN_HEADER4, true);
            this->StyleSetBold(wxSTC_MARKDOWN_HEADER5, true);
            this->StyleSetBold(wxSTC_MARKDOWN_HEADER6, true);

            /*// Размер заголовков убывает от H1 к H6 (базовый размер текста — 11pt).
            this->StyleSetSize(wxSTC_MARKDOWN_HEADER1, 20);
            this->StyleSetSize(wxSTC_MARKDOWN_HEADER2, 17);
            this->StyleSetSize(wxSTC_MARKDOWN_HEADER3, 15);
            this->StyleSetSize(wxSTC_MARKDOWN_HEADER4, 13);
            this->StyleSetSize(wxSTC_MARKDOWN_HEADER5, 12);
            this->StyleSetSize(wxSTC_MARKDOWN_HEADER6, 11);*/

            this->StyleSetForeground(wxSTC_MARKDOWN_LINK,    wxColour("#0B63CE"));
            this->StyleSetForeground(wxSTC_MARKDOWN_CODE,    wxColour("#8B5A2B"));
            this->StyleSetForeground(wxSTC_MARKDOWN_CODEBK,  wxColour("#8B5A2B"));
            break;

        case wxSTC_LEX_XML:
            // Подсветка XML-разметки
            this->StyleSetForeground(wxSTC_H_TAG,          wxColour("#0B63CE")); // <tag>
            this->StyleSetForeground(wxSTC_H_TAGUNKNOWN,   wxColour("#0B63CE"));
            this->StyleSetForeground(wxSTC_H_ATTRIBUTE,    wxColour("#8B5A2B")); // имя атрибута
            this->StyleSetForeground(wxSTC_H_ATTRIBUTEUNKNOWN, wxColour("#8B5A2B"));
            this->StyleSetForeground(wxSTC_H_DOUBLESTRING, wxColour("#1A7F37")); // "значение"
            this->StyleSetForeground(wxSTC_H_SINGLESTRING, wxColour("#1A7F37")); // 'значение'
            this->StyleSetForeground(wxSTC_H_COMMENT,      wxColour("#6A737D")); // <!-- -->
            this->StyleSetForeground(wxSTC_H_ENTITY,       wxColour("#CF222E")); // &amp;
            this->StyleSetForeground(wxSTC_H_XMLSTART,     wxColour("#6A737D")); // <?xml
            this->StyleSetForeground(wxSTC_H_XMLEND,       wxColour("#6A737D")); // ?>
            break;

        default:
            break;
    }

    // Перекрасить весь буфер под новый лексер.
    this->Colourise(0, -1);
}

void hmbText::on_text_change(wxStyledTextEvent& event)
{
    if(this->on_edit)
    {
        if(this->toolsbar) this->toolsbar->save_btn_enable(true);
        wx_to_utf8(this->GetText(), HMB_SRC_DATA);
    }
    event.Skip();
}

// Загрузка в виджет данных из текстовой строки
void hmbText::load_data(const std::string& data)
{
    bool t = this->on_edit;
    wxString wx_data = wxString::FromUTF8(data.data(), data.size());
    
    this->on_edit = false;
    this->ClearAll();
    this->SetValue(wx_data);
    this->EmptyUndoBuffer();
    this->SetSavePoint();
    this->on_edit = t;
}
