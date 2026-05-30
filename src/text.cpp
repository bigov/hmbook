#include "hmb/text.h"

hmbText::hmbText(wxWindow* parent)
    : wxStyledTextCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 
        wxBORDER_NONE | wxVSCROLL )
{
    // Базовый стиль текста для всего буфера STC.
    this->StyleSetFont(wxSTC_STYLE_DEFAULT, HMB_FONT_BASE);
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

    this->SetLexer(wxSTC_LEX_MARKDOWN);

    // Минимальная подсветка markdown
    this->StyleSetForeground(wxSTC_MARKDOWN_HEADER1, wxColour("#000000"));
    this->StyleSetForeground(wxSTC_MARKDOWN_HEADER2, wxColour("#000000"));
    this->SetProperty("lexer.markdown.header.eolfill", "1");
    
    this->StyleSetForeground(wxSTC_MARKDOWN_LINK,    wxColour("#0B63CE"));
    this->StyleSetForeground(wxSTC_MARKDOWN_CODE,    wxColour("#8B5A2B"));
    this->StyleSetForeground(wxSTC_MARKDOWN_CODEBK,  wxColour("#8B5A2B"));

    this->Colourise(0, -1);
}
