#ifndef HMB_TEXT_H
#define HMB_TEXT_H

#include "wx/textctrl.h"
#include "tools.h"

class hmbText: public wxTextCtrl
{
    public:
    explicit hmbText(wxWindow* parent, long extraStyle = 0);
        //~hmbText() override;
};

#endif // HMB_TEXT_H