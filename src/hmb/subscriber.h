#ifndef HMB_SUBSCRIBER_H
#define HMB_SUBSCRIBER_H

#include "wx/string.h"

// Абстрактный класс подписчика - любой класс может стать подписчиком, если реализует load_file(...).
// источник события знает только интерфейс, а не реализацию.
class hmbSubscriber
{
public:
    // Виртуальный деструктор обязателен для корректного удаления через базовый указатель.
    virtual ~hmbSubscriber() = default;
    // Абстрактный метод, который должен быть реализован подписчиком.
    virtual void load_file(const wxString& filePath) = 0; // filePath - полный путь к выбранному файлу.
};

#endif // HMB_SUBSCRIBER_H