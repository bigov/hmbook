#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/config.h>
#include "window.h"

class MyApp: public wxApp
{
public:
    bool OnInit() override;
};

wxIMPLEMENT_APP(MyApp);


bool MyApp::OnInit()
{
    if ( !wxApp::OnInit() ) return false;
    hmbWindow* win = new hmbWindow("Hyper Markdown Book");
    win->Show(true);

    return true;
}
