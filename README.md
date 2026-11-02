# Aurora Player

Aurora Player — локальный музыкальный плеер на Qt 6/QML с тёмным адаптивным интерфейсом. Основная платформа разработки — Linux/Hyprland; используемые Qt API также подходят для сборки под Windows и Android.

## Возможности

- воспроизведение MP3, FLAC, WAV, OGG, Opus, M4A и других форматов, поддерживаемых Qt Multimedia;
- локальная SQLite-библиотека, поиск по названию, исполнителю и альбому;
- импорт отдельных файлов и рекурсивное сканирование папок;
- импорт по ссылкам YouTube, Яндекс Музыки и VK через `yt-dlp`;
- импорт ссылок Spotify через `spotdl`;
- выбор качества: исходное/лучшее, MP3 192 kbps или MP3 128 kbps;
- рейтинг треков, счётчик прослушиваний, данные о длительности, формате и битрейте;
- сохранение громкости, качества загрузки и папки библиотеки между запусками.

Удаление трека из библиотеки не удаляет аудиофайл с диска.

## Зависимости

Для сборки нужны CMake 3.21+, компилятор C++20 и Qt 6 с модулями Core, Gui, Widgets, Qml, Quick, QuickControls2, Sql и Multimedia.

Для чтения метаданных и перекодирования используется FFmpeg (`ffprobe`/`ffmpeg`). Для сетевого импорта дополнительно установите нужные загрузчики:

```bash
python3 -m pip install --user yt-dlp
python3 -m pip install --user spotdl
```

Приложение ищет команды в `PATH`. Если загрузчик не найден, в диалоге импорта отображается подсказка. Путь можно задать в настройках `ytDlpPath` и `spotdlPath` через `QSettings`.

Приложение не обходит DRM, ограничения подписки или защищённый кэш. Конкретная ссылка загрузится только если её поддерживает соответствующий внешний загрузчик и контент доступен пользователю. Пользователь отвечает за соблюдение условий сервиса и авторских прав.

## Сборка и запуск

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j2
./build/AuroraPlayer
```

SQLite создаётся в стандартной директории данных Qt. По умолчанию музыка копируется в `~/Music/Aurora Player`; уже находящиеся в папке библиотеки файлы не копируются повторно.

Для проверки запуска без графического сервера:

```bash
QT_QPA_PLATFORM=offscreen ./build/AuroraPlayer --smoke-test
```

Можно одновременно проверить импорт файла:

```bash
QT_QPA_PLATFORM=offscreen ./build/AuroraPlayer --smoke-test --import /path/to/track.mp3
```

## Структура

```text
player/
├── CMakeLists.txt
├── src/
│   ├── app/main.cpp
│   └── core/
│       ├── librarymodel.*
│       ├── playbackcontroller.*
│       ├── importmanager.*
│       ├── downloadmanager.*
│       └── settingsstore.*
├── qml/
│   ├── Main.qml
│   ├── Theme.qml
│   ├── GlassCard.qml
│   └── GlassButton.qml
└── docs_task.md
```

Сетевые рекомендации, синхронизация между устройствами и готовый Android-пакет пока не реализованы.
