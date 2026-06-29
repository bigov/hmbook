#include "tools_bar.h"
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/artprov.h>
#include <wx/bmpbndl.h>
#include <wx/image.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>

namespace
{
// Возвращает набор изображений из файла, путь к которому задан относительно
// каталога с исполняемым модулем приложения (куда сборка копирует ресурсы).
wxBitmapBundle icon_from_file(const wxString& relativePath)
{
    // Однократная регистрация обработчика PNG.
    if ( !wxImage::FindHandler(wxBITMAP_TYPE_PNG) )
        wxImage::AddHandler(new wxPNGHandler);

    const wxFileName exeDir(wxStandardPaths::Get().GetExecutablePath());
    wxFileName file(relativePath);
    file.MakeAbsolute(exeDir.GetPath());   // относительно каталога exe

    return wxBitmapBundle::FromBitmap(
        wxBitmap(file.GetFullPath(), wxBITMAP_TYPE_PNG));
}

// Возвращает осветлённую (приглушённую) версию значка для неактивного
// состояния кнопки. Чем больше brightness, тем светлее результат.
wxBitmapBundle dimmed_icon(const wxBitmapBundle& src, unsigned char brightness = 255)
{
    wxImage img = src.GetBitmap(wxSize(16, 16)).ConvertToImage();
    return wxBitmapBundle::FromBitmap(wxBitmap(img.ConvertToDisabled(brightness)));
}
} // namespace


hmbToolsBar::hmbToolsBar(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
{
    this->init_toolsbar();
    this->save_btn_enable(false);
    this->text_mode_enable(false);
    this->wrap_btn_enable(false);
}

void hmbToolsBar::init_toolsbar()
{
    SetBackgroundColour(*wxWHITE);

    wxPanel* topBorder = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 1));
    wxPanel* bottomBorder = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 1));
    topBorder->SetBackgroundColour("#AAAAAA");
    bottomBorder->SetBackgroundColour("#AAAAAA");

    toolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxTB_HORIZONTAL | wxTB_FLAT | wxTB_TEXT | wxTB_NODIVIDER);
    toolbar->SetBackgroundColour(*wxWHITE);
    toolbar->SetToolBitmapSize(wxSize(16, 16));

    toolbar->AddTool(wxID_OPEN, wxEmptyString, wxArtProvider::GetBitmapBundle(wxART_FOLDER_OPEN, wxART_TOOLBAR),
        _("Open dir [Ctrl+O]"), wxITEM_NORMAL);

    const wxBitmapBundle saveIcon = icon_from_file("images/icons16/wxID_SAVE.png");
    toolbar->AddTool(wxID_SAVE, wxEmptyString,
        saveIcon,                 // активная — контрастная
        dimmed_icon(saveIcon),    // неактивная — осветлённая
        wxITEM_NORMAL,
        _("Save file [Ctrl+S]"));

    toolbar->AddSeparator();

    toolbar->AddTool(wxID_VIEW_DETAILS, wxEmptyString, wxArtProvider::GetBitmapBundle(wxART_HELP_PAGE, wxART_TOOLBAR),
        _("Debug biffer [Ctrl+D]"), wxITEM_CHECK);

    toolbar->AddTool(HMB_ID_WRAP, wxEmptyString, wxArtProvider::GetBitmapBundle(wxART_LIST_VIEW, wxART_TOOLBAR),
        _("Word wrap"), wxITEM_CHECK);
    toolbar->ToggleTool(HMB_ID_WRAP, true); // перенос слов включён по умолчанию

    toolbar->AddSeparator();

    toolbar->AddTool(wxID_EXIT, wxEmptyString, wxArtProvider::GetBitmapBundle(wxART_QUIT, wxART_TOOLBAR),
        _("CLose app [Ctrl+W]"), wxITEM_NORMAL);


    toolbar->Realize();

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(topBorder, 0, wxEXPAND);
    sizer->Add(toolbar, 0, wxEXPAND | wxLEFT, 8);
    sizer->Add(bottomBorder, 0, wxEXPAND);
    SetSizerAndFit(sizer);
}


void hmbToolsBar::save_btn_enable(bool state)
{
     toolbar->EnableTool(wxID_SAVE, state);
}

void hmbToolsBar::text_mode_enable(bool state)
{
     toolbar->EnableTool(wxID_VIEW_DETAILS, state);
}

void hmbToolsBar::wrap_btn_enable(bool state)
{
     toolbar->EnableTool(HMB_ID_WRAP, state);
}