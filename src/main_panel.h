#pragma once

#include "wx/panel.h"

class TxtRich;
class wxAuiNotebook;
class wxTextCtrl;

class MainPanel : public wxPanel
{
public:
    explicit MainPanel(wxWindow* parent);

    TxtRich* get_txt_rich() const;

private:
    wxAuiNotebook* m_book;
    TxtRich* m_txtRich;
    wxTextCtrl* m_source;
    wxTextCtrl* m_buffer;
};
