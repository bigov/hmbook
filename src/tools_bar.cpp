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
typedef  struct {
    wxBitmapBundle normal;
    wxBitmapBundle dimmed;
} icons_array;

const wxString icons_dir = "images/svg/";

// Возвращает набор изображений из SVG-файла, путь к которому задан относительно
// каталога с исполняемым модулем приложения (куда сборка копирует ресурсы).
icons_array get_icon(const wxString& fname)
{
    icons_array result;
    const wxFileName exeDir(wxStandardPaths::Get().GetExecutablePath());

    wxString color_9b = "#9a9a9a";
    wxString color_4b = "#4b4b4b";
    wxString color_dimmed = "#ccccff";

    wxFileName file(icons_dir + fname + ".svg");
    file.MakeAbsolute(exeDir.GetPath());

    std::string svg = "";
    file_read(file.GetFullPath(), svg);
    wxString data = wxString::FromUTF8(svg);

    result.normal = wxBitmapBundle::FromSVG(data, wxSize(16, 16));

    data.Replace(color_9b, color_dimmed);
    data.Replace(color_4b, color_dimmed);
    result.dimmed = wxBitmapBundle::FromSVG(data, wxSize(16, 16));
    
    return result;
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

    // Разделитель для кнопок (по горизонтали)
    auto* spacer = new wxControl(toolbar, wxID_ANY, wxDefaultPosition, wxSize(1, 1), wxBORDER_NONE);
    spacer->SetBackgroundColour(*wxWHITE);


    auto icon = get_icon("OPEN");
    toolbar->AddTool(wxID_OPEN, wxEmptyString, icon.normal, icon.dimmed, wxITEM_NORMAL, _("Open dir [Ctrl+O]"));
    toolbar->AddControl(spacer);

    icon = get_icon("SAVE");
    toolbar->AddTool(wxID_SAVE, wxEmptyString, icon.normal, icon.dimmed, wxITEM_NORMAL, _("Save file [Ctrl+S]"));
    toolbar->AddControl(spacer);

    icon = get_icon("SHOW_BUFFER_XML");
    toolbar->AddTool(hmbID_SHOW_BUFFER_XML, wxEmptyString, icon.normal, icon.dimmed, wxITEM_CHECK, _("Debug biffer [Ctrl+D]"));
    toolbar->AddControl(spacer);

    icon = get_icon("LINE_WRAPPING");
    toolbar->AddTool(hmbID_LINE_WRAPPING, wxEmptyString, icon.normal, icon.dimmed, wxITEM_CHECK, _("Word wrap"));


    toolbar->EnableTool(wxID_SAVE, false);
    toolbar->EnableTool(hmbID_SHOW_BUFFER_XML, false);
    toolbar->EnableTool(hmbID_LINE_WRAPPING, false);
    toolbar->ToggleTool(hmbID_LINE_WRAPPING, false);

    // Растягивающийся разделитель: занимает всё свободное место и
    // прижимает следующую за ним кнопку (Exit) к правому краю панели.
    toolbar->AddStretchableSpace();

    icon = get_icon("EXIT");
    toolbar->AddTool(wxID_EXIT, wxEmptyString, icon.normal, icon.dimmed, wxITEM_NORMAL, _("CLose app [Ctrl+W]"));

    toolbar->Realize();

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(topBorder, 0, wxEXPAND);
    sizer->AddSpacer(2);   // зазор 2px между верхней границей и кнопками
    sizer->Add(toolbar, 0, wxEXPAND | wxLEFT | wxRIGHT, 8);
    sizer->Add(bottomBorder, 0, wxEXPAND);
    SetSizerAndFit(sizer);
}


void hmbToolsBar::save_btn_enable(bool state)
{
     toolbar->EnableTool(wxID_SAVE, state);
}

void hmbToolsBar::text_mode_enable(bool state)
{
     toolbar->EnableTool(hmbID_SHOW_BUFFER_XML, state);
}

void hmbToolsBar::wrap_btn_enable(bool state)
{
     toolbar->EnableTool(hmbID_LINE_WRAPPING, state);
}