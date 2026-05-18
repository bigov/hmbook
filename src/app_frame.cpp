#include "wx/sstream.h"
#include "wx/filedlg.h"
#include "wx/filename.h"
#include "wx/wfstream.h"
#include "wx/log.h"
#include "wx/filefn.h"
#include "wx/icon.h"
#include "wx/image.h"
#include "wx/menu.h"
#include "wx/panel.h"
#include "wx/sizer.h"
#include "wx/stdpaths.h"
#include "wx/config.h"
#include "wx/dirdlg.h"
#include "wx/dir.h"

#include "app_frame.h"
#include "main_panel.h"
#include "nav_panel.h"

static const int APP_CLOSE = 1000;
static const wxString ASSETS_DIR = "assets";
static const wxString APP_ICON_FNAME = "icon.png";

AppFrame::AppFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL)
{
    wxInitAllImageHandlers();
    
    // Настройка иконки приложения
    const wxString iconPath = wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPathWithSep() 
        + ASSETS_DIR + wxFileName::GetPathSeparator() + APP_ICON_FNAME;
    SetAppIcon(iconPath);

    this->splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition,
                                    wxDefaultSize, wxSP_NO_XP_THEME);
    
    MainPanel* main_panel = new MainPanel(this->splitter);
    NavPanel* nav_panel = new NavPanel(this->splitter, main_panel->get_txt_rich());
    this->txt_rich = main_panel->get_txt_rich();
    this->tree_viewer = nav_panel->get_tree_viewer();
    
    this->splitter->SplitVertically(nav_panel, main_panel);
    this->splitter->SetMinimumPaneSize(80); // Минимальная ширина
    this->splitter->SetSashGravity(0.0);    // При изменении размера окна ширина панели навигации остается неизменной.

    // Корневой компоновщик фрейма: размещает splitter на все окно, оставляя
    
    wxBoxSizer* frameSizer = new wxBoxSizer(wxHORIZONTAL);
    frameSizer->Add(this->splitter, 1, wxEXPAND | wxLEFT | wxRIGHT, 3); // боковые внешние поля
    
    this->SetSizer(frameSizer);
    this->SetBackgroundColour(this->splitter->GetBackgroundColour());

    wxMenu* fileMenu = new wxMenu;

    fileMenu->Append(wxID_OPEN, _("Open Dir\tCtrl+O"));
    Bind(wxEVT_MENU, &AppFrame::OpenDir, this, wxID_OPEN);

    fileMenu->Append(wxID_SAVEAS, _("Save As...\tCtrl+S"));
    Bind(wxEVT_MENU, &AppFrame::FileSaveAs, this, wxID_SAVEAS);

    fileMenu->Append(APP_CLOSE, _("Exit\tCtrl+W"));
    Bind(wxEVT_MENU, &AppFrame::OnClose, this, APP_CLOSE);
    Bind(wxEVT_CLOSE_WINDOW, &AppFrame::OnWindowClose, this);
    
    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, _("File"));
    menuBar->Append(main_panel->get_txt_rich()->edit_menu(), _("Edit"));
    SetMenuBar(menuBar);

#if wxUSE_STATUSBAR
    CreateStatusBar();
#endif // wxUSE_STATUSBAR

    load_params();
}

void AppFrame::SetAppIcon(const wxString& iconPath)
{
    // Set the application icon
    if (wxFileExists(iconPath))
    {
        wxIcon appIcon;
        appIcon.CopyFromBitmap(wxBitmap(iconPath, wxBITMAP_TYPE_PNG));
        if (appIcon.IsOk())
        {
            SetIcon(appIcon);
        }
    }
}


void AppFrame::OpenDir(wxCommandEvent& WXUNUSED(event))
{
    wxString defaultDir = wxGetHomeDir(); // начальная папка
    wxDirDialog dlg(this, "Select directory", defaultDir, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    if (dlg.ShowModal() == wxID_OK) {
        wxString path = dlg.GetPath();
        this->tree_viewer->load_directory(path);
    }
}


void AppFrame::FileSaveAs(wxCommandEvent& WXUNUSED(event))
{
    wxFileDialog
        saveFileDialog(this, _("Save file as"), "", "",
                      "Plain text files (*.txt)|*.txt|Rich text XML (*.wxrt)|*.wxrt",
                       wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL) return;
    
    const int fileType = saveFileDialog.GetFilterIndex();
    wxFileName fileName(saveFileDialog.GetPath());
    if (fileName.GetExt().IsEmpty())
    {
        fileName.SetExt(fileType == 1 ? RICH_BUFFER_EXT : TEXT_BUFFER_EXT);
    }
    
    const wxString filePath = fileName.GetFullPath();

    if (fileType == 0) txt_rich->save_plain_file(filePath);
    if (fileType == 1) txt_rich->save_xml_file(filePath);
}


void AppFrame::OnClose(wxCommandEvent& WXUNUSED(event))
{
    Close(true);
}

void AppFrame::OnWindowClose(wxCloseEvent& event)
{
    save_params();
    event.Skip();
}

void AppFrame::load_params()
{
    wxConfig config("Book", "Hyper-Markdown");
    /* Методы класса wxConfig:  ReadLong, ReadBool, ReadDouble и общего Read для wxString.
      wxString name;
      config.Read("/User/Name", &name, "");
      double volume = config.ReadDouble("/User/Volume", 1.0);
      bool maximized = config.ReadBool("/MainWindow/Maximized", false);
     */

    int screenWidth = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
    int screenHeight = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y);

    long width = (long)(screenWidth * 0.8); // 80% от ширины экрана
    long height = (long)(screenHeight * 0.8); // 80% от высоты экрана
    long x = 10, y = 10, d = 250;

    // Чтение со значениями по умолчанию
    int win_width  = (int)config.ReadLong("/MainWindow/Width", width);
    int win_height = (int)config.ReadLong("/MainWindow/Height", height);
    int win_x = (int)config.ReadLong("/MainWindow/X", x);
    int win_y = (int)config.ReadLong("/MainWindow/Y", y);
    int span = (int)config.ReadLong("/MainWindow/span", d);

    this->SetSize(win_x, win_y, win_width, win_height);
    this->splitter->SetSashPosition(span); // позиция разделителя
    this->tree_viewer->load_directory(config.Read("/MainWindow/current_dir", wxEmptyString));
    this->txt_rich->load_file(config.Read("/MainWindow/current_file", wxEmptyString));
}

void AppFrame::save_params()
{
    wxConfig config("Book", "Hyper-Markdown");

    const wxSize size = GetSize();
    const wxPoint pos = GetPosition();

    config.Write("/MainWindow/Width", (long)size.GetWidth());
    config.Write("/MainWindow/Height", (long)size.GetHeight());
    config.Write("/MainWindow/X", (long)pos.x);
    config.Write("/MainWindow/Y", (long)pos.y);
    config.Write("/MainWindow/span", (long)this->splitter->GetSashPosition());
    config.Write("/MainWindow/current_dir", this->tree_viewer->current_dir);
    config.Write("/MainWindow/current_file", this->txt_rich->current_filePath);
    config.Flush();
}
