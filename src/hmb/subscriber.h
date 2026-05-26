#ifndef HMB_SUBSCRIBER_H
#define HMB_SUBSCRIBER_H

#include "wx/string.h"

// Абстрактный класс подписчика. Источник события знает только интерфейс.
class hmbSubscriber
{
public:
    // Виртуальный деструктор обязателен для корректного удаления через базовый указатель.
    virtual ~hmbSubscriber() = default;
    // Интерфейс - абстрактный метод, который должен быть реализован подписчиком
    virtual void load_file(const wxString& filePath) = 0; // filePath - полный путь к выбранному файлу.
};

#endif // HMB_SUBSCRIBER_H