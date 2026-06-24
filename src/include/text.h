#ifndef HMB_TEXT_H
#define HMB_TEXT_H

#include <wx/textctrl.h>
#include <wx/stc/stc.h>
#include "tools.h"
#include "tools_bar.h"

class hmbText: public wxStyledTextCtrl
{
  public:
    explicit hmbText(wxWindow* parent);
    void load_data(const std::string& data);
    bool on_edit = false;

    // Переключить используемый лексер подсветки синтаксиса (wxSTC_LEX_*)
    // и применить соответствующее ему оформление стилей.
    void set_lexer(int lexer);

    hmbToolsBar* toolsbar = nullptr;

  private:
    void on_text_change(wxStyledTextEvent& event);
};

#endif // HMB_TEXT_H