#ifndef HMB_TEXT_H
#define HMB_TEXT_H

#include <wx/textctrl.h>
#include <wx/stc/stc.h>
#include "tools.h"

class hmbText: public wxStyledTextCtrl
{
    public:
    explicit hmbText(wxWindow* parent);
};

#endif // HMB_TEXT_H