#include "window.h"
#include "tools.h"

wxString HMB_FNAME = wxEmptyString;
wxString HMB_DNAME = wxEmptyString;

wxFont HMB_FONT_BASE = wxFontInfo(11).FaceName("Adwaita Sans Text");
wxFont HMB_FONT_MONO = wxFontInfo(11).FaceName("Adwaita Mono");

wxColor HMB_COLOR_BASE_FG = "#444444";
wxColor HMB_COLOR_BASE_BG = "#ffffff";

static const int APP_CLOSE = 1000;
static const wxString ASSETS_DIR = "assets";
static const wxString APP_ICON_FNAME = "icon.png";

hmbWindow::hmbWindow(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL)
{
    wxInitAllImageHandlers();
    
    // Настройка иконки приложения
    const wxString iconPath = wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPathWithSep() 
        + ASSETS_DIR + wxFileName::GetPathSeparator() + APP_ICON_FNAME;
    SetAppIcon(iconPath);

    this->splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_NO_XP_THEME);
    this->splitter->SetMinimumPaneSize(80); // Минимальная ширина
    this->splitter->SetSashGravity(0.0);    // При изменении размера окна ширина панели навигации остается неизменной.
    
    this->panel_view = new hmbPanelView(this->splitter);
    this->panel_tree = new hmbPanelTree(this->splitter);
    this->panel_tree->bind_subscriber(this->panel_view); // связать выбор в дереве с отображением в панели просмотра
    
    this->splitter->SplitVertically(panel_tree, panel_view);

    // Корневой компоновщик фрейма: размещает splitter на все окно
    wxBoxSizer* frameSizer = new wxBoxSizer(wxHORIZONTAL);
    frameSizer->Add(this->splitter, 1, wxEXPAND | wxLEFT | wxRIGHT, 3); // боковые внешние поля
    
    this->SetSizer(frameSizer);
    this->SetBackgroundColour(this->splitter->GetBackgroundColour());

    wxMenu* fileMenu = new wxMenu;

    fileMenu->Append(wxID_OPEN, _("Open Dir\tCtrl+O"));
    Bind(wxEVT_MENU, [this](wxCommandEvent& WXUNUSED(event)) {this->panel_tree->open_dir();}, wxID_OPEN);

    fileMenu->Append(wxID_SAVEAS, _("Save As...\tCtrl+S"));
    Bind(wxEVT_MENU, [this](wxCommandEvent& WXUNUSED(event)) {this->panel_view->save_file_as();}, wxID_SAVEAS);

    fileMenu->Append(APP_CLOSE, _("Exit\tCtrl+W"));
    Bind(wxEVT_MENU, &hmbWindow::OnClose, this, APP_CLOSE);
    Bind(wxEVT_CLOSE_WINDOW, &hmbWindow::OnWindowClose, this);
    
    this->menuBar = new wxMenuBar;
    this->menuBar->Append(fileMenu, _("File"));
    this->menuBar->Append(panel_view->edit_menu(), _("Edit"));
    this->SetMenuBar(this->menuBar);

    this->statusBar = new hmbStatusBar(this);
    this->SetStatusBar(this->statusBar);
    this->statusBar->set_text_2(title);
    this->panel_view->bind_subscriber(this->statusBar);

    load_params();
}

// Set the application icon from the specified path if it exists and is a valid icon file.
void hmbWindow::SetAppIcon(const wxString& iconPath)
{
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

void hmbWindow::OnClose(wxCommandEvent& WXUNUSED(event))
{
    Close(true);
}

void hmbWindow::OnWindowClose(wxCloseEvent& event)
{
    save_params();
    event.Skip();
}

void hmbWindow::load_params()
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
    this->panel_tree->set_root_dir(config.Read("/MainWindow/current_dir", wxEmptyString));
    auto current_file = config.Read("/MainWindow/current_file", wxEmptyString);
    // Установка курсора на указанный файл (если он существует) автоматически приведет к его загрузке в панель просмотра.
    this->panel_tree->set_cursor_to(current_file);
}   

void hmbWindow::save_params()
{
    wxConfig config("Book", "Hyper-Markdown");

    const wxSize size = GetSize();
    const wxPoint pos = GetPosition();

    config.Write("/MainWindow/Width", (long)size.GetWidth());
    config.Write("/MainWindow/Height", (long)size.GetHeight());
    config.Write("/MainWindow/X", (long)pos.x);
    config.Write("/MainWindow/Y", (long)pos.y);
    config.Write("/MainWindow/span", (long)this->splitter->GetSashPosition());
    config.Write("/MainWindow/current_dir", HMB_DNAME);
    config.Write("/MainWindow/current_file", HMB_FNAME);
    config.Flush();
}
