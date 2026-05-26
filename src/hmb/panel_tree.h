#ifndef HMB_PANEL_TREE_H
#define HMB_PANEL_TREE_H

#include "wx/dirdlg.h"
#include "wx/panel.h"
#include "wx/sizer.h"

#include "subscriber.h"

class hmbTree;

class hmbPanelTree : public wxPanel
{
public:
    explicit hmbPanelTree(wxWindow* parent);
    ~hmbPanelTree();

    // Регистрация подписчика на выбор файла в дереве.
    // Подписчик должен жить не меньше, чем hmbPanelTree, пока активна подписка.
    // Передача nullptr - отключение подписки.
    void bind_subscriber(hmbSubscriber* subscriber);
    wxString get_current_dir() const;
    void open_dir();
    void load_directory(const wxString& dir);

private:
    // Внутренний контрол дерева файлов.
    hmbTree* tree_viewer = nullptr;
};

#endif // HMB_PANEL_TREE_H