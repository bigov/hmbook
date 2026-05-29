#include "hmb/text.h"

hmbText::hmbText(wxWindow* parent, long extraStyle)
    : wxTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
        wxBORDER_NONE | wxTE_MULTILINE | wxTE_RICH2 | extraStyle)
{
    this->SetFont(HMB_FONT_BASE);
    this->SetBackgroundColour(HMB_COLOR_BASE_BG);
    this->SetForegroundColour(HMB_COLOR_BASE_FG);

    wxTextAttr attr;
    attr.SetLeftIndent(8);
    attr.SetRightIndent(8);
    this->SetDefaultStyle(attr);
}
