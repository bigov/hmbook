# Назначение клавиш для сборки и запуска

## Обзор

Стандартное поведение VS Code переназначено для удобства повседневной разработки:

| Клавиша      | Действие                                  |
|--------------|-------------------------------------------|
| **F5**       | Сборка (Debug) + запуск **без отладки**   |
| **Ctrl+F5**  | Сборка (Debug) + запуск **с отладчиком**  |

## Расположение файла конфигурации

Привязки клавиш хранятся в пользовательском файле VS Code:

```
%APPDATA%\Code\User\keybindings.json
```

> Этот файл является глобальной настройкой VS Code и не входит в репозиторий проекта.

## Содержимое keybindings.json

```json
[
  {
    "key": "f5",
    "command": "-workbench.action.debug.start"
  },
  {
    "key": "f5",
    "command": "workbench.action.debug.run",
    "when": "!inDebugMode"
  },
  {
    "key": "ctrl+f5",
    "command": "-workbench.action.debug.run"
  },
  {
    "key": "ctrl+f5",
    "command": "workbench.action.debug.start",
    "when": "!inDebugMode"
  }
]
```

## Пояснение записей

| Запись | Назначение |
|--------|------------|
| `"-workbench.action.debug.start"` на **F5** | Безусловно снимает стандартную привязку «запуск с отладкой» с F5 |
| `"workbench.action.debug.run"` на **F5** | Назначает «запуск без отладки» на F5 |
| `"-workbench.action.debug.run"` на **Ctrl+F5** | Безусловно снимает стандартную привязку «запуск без отладки» с Ctrl+F5 |
| `"workbench.action.debug.start"` на **Ctrl+F5** | Назначает «запуск с отладкой» на Ctrl+F5 |

## Установка вручную

1. Открыть палитру команд: `Ctrl+Shift+P`.
2. Ввести: **Preferences: Open Keyboard Shortcuts (JSON)**.
3. Вставить содержимое из раздела выше.
4. Сохранить файл — привязки применяются немедленно.