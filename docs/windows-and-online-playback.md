# Windows и подключение онлайн-аудио

## Сборка на Windows

Нужны CMake 3.21+, Ninja, Windows-комплект Qt **6.11 или новее** и соответствующий компилятор. Установи Qt Multimedia и Qt Shader Tools; Quick, QuickControls2, QuickShapes, Sql/QSQLITE, Network и Concurrent должны находиться в том же комплекте. Нельзя смешивать библиотеки MSVC, GCC MinGW и LLVM MinGW.

Для Qt MinGW, из PowerShell в каталоге проекта (пути замени на свои):

```powershell
./scripts/build-windows.ps1 -QtRoot C:/Qt/6.11.1/mingw_64 -CompilerBin C:/Qt/Tools/mingw1310_64/bin
```

Для MSVC запусти PowerShell из Developer Command Prompt с настроенным окружением компилятора и передай `-QtRoot C:/Qt/6.11.1/msvc2022_64`.

Скрипт собирает Release и выполняет Qt deployment: в `dist/windows/bin/AuroraPlayer.exe` и соседние каталоги попадают DLL, QML-модули и плагины, включая SQLite и Multimedia. Переноси **весь `dist/windows`**, запускай `bin/AuroraPlayer.exe`. FFmpeg (`ffmpeg.exe`, `ffprobe.exe`), yt-dlp и JS-runtime для внешних загрузчиков устанавливаются отдельно из настроек библиотеки либо через PATH; инструменты рядом с `.exe` также обнаруживаются. DLL FFmpeg, используемые Qt Multimedia, и консольные `ffmpeg.exe`/`ffprobe.exe` — разные зависимости.

Альтернативные команды:

```powershell
cmake -S . -B build/windows-native -G Ninja -DCMAKE_PREFIX_PATH=C:/Qt/6.11.1/mingw_64 -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DAURORA_WITH_WEBENGINE=OFF
cmake --build build/windows-native --parallel 4
cmake --install build/windows-native --prefix dist/windows --config Release
```

При смене комплекта используй новый каталог сборки или `cmake --fresh`: кэш хранит прежние пути и компилятор. `BUILD_TESTING=OFF` позволяет собирать приложение без Qt Test.

## Почему возникало сообщение об отсутствующих пакетах

Проект требовал Qt 6.12 при установленных Windows-комплектах 6.11.1. Минимальная версия теперь 6.11. WebEngine стал необязательным: если модуль отсутствует, приложение автоматически использует установленный Chromium-браузер. Это особенно важно для MinGW: [Qt WebEngine не собирается этим компилятором](https://doc.qt.io/qt-6.8/qtwebengine-platform-notes.html).

`-DAURORA_WITH_WEBENGINE=OFF` полностью исключает WebEngine из C++, QML и зависимостей приложения. Вход и работа с сервисами идут через Chrome, Edge, Brave или Яндекс Браузер. При наличии WebEngine и включённой опции встроенный браузер сохраняется. Проверка Windows PATH использует `;`, добавлен поиск Яндекс Браузера и Windows Python в управляемом venv.

## Кросс-сборка из Linux

В репозитории есть `cmake/toolchains/windows-llvm-mingw.cmake`. По умолчанию он использует `~/Qt/llvm-mingw`, `~/Qt/6.11.1/gcc_64` и `~/Qt/6.11.1/llvm-mingw_64`. Параметры `LLVM_MINGW_ROOT`, `AURORA_QT_ROOT`, `AURORA_QT_VERSION`, `QT_HOST_PATH`, `CMAKE_PREFIX_PATH` переопределяются через `-D`.

```bash
cmake -S . -B build/windows -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/windows-llvm-mingw.cmake \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DAURORA_WITH_WEBENGINE=OFF
cmake --build build/windows --parallel 4
./scripts/package-windows.sh build/windows/AuroraPlayer.exe
```

Для ShaderTools нужен **Linux `qsb`**: Windows `qsb.exe` нельзя запускать на Linux. CMake ищет его только в host Qt, а если он отсутствует — выдаёт понятную ошибку. Совместимый инструмент можно указать явно: `-DAURORA_HOST_QSB=/path/to/linux/qsb`. В текущем окружении установлен системный `/usr/lib/qt6/bin/qsb` версии 6.12; его формат QSB v9 совпадает с форматом установленного Qt 6.11.1. Для проверки здесь использован этот явный параметр. Остальные инструменты берутся из host Qt 6.11.1.

Пакет для переноса после кросс-сборки: `dist/windows/AuroraPlayer.exe`, плагины, QML и DLL в той же папке. Для диагностики — `run-debug.bat`. Скрипт упаковки предназначен для LLVM MinGW; для нативной Windows-сборки используй `cmake --install`.

## Онлайн-прослушивание

- Общая панель над плеером показывает текущий этап, источник, номер попытки для альтернативы, прошедшие секунды и отмену. Во время поиска ссылки полоса неопределённая: сервисы не сообщают процент поиска.
- Когда Qt получает аудио, отображается **процент заполнения буфера воспроизведения**, а не процент скачивания всего трека. Данные берутся из [QMediaPlayer::bufferProgress](https://doc.qt.io/qt-6.8/qmediaplayer.html#bufferProgress-prop). Начать слушать можно до загрузки полного файла.
- Альтернативный источник можно подключить через 1,5 секунды вместо 4,5; поиск альтернатив идёт параллельно исходному подключению. Совпадение названия, исполнителя и длительности проверяется прежними правилами.
- Не более двух попыток на альтернативный сервис; общий срок подключения с подбором и буферизацией — 45 секунд. На отдельное получение потока в разделе поиска — 20 секунд. Зависшее подключение не продолжает бесконечный перебор.
- Ссылки потоков хранятся в памяти до двух минут (максимум 64 записи). Повторное включение сокращает обращения к сервису. Ошибка удаляет соответствующую ссылку из кэша; метаданные и идентификатор новой очереди сохраняются. Кэш HomeFeed сбрасывается при изменении сессии аккаунта.
- yt-dlp получает URL без скачивания и конвертации; FFmpeg/ffprobe больше не блокируют этот этап. Уменьшены его повторы и сетевое ожидание. Скачивание в медиатеку остаётся отдельной операцией.

Реальная скорость и доступность зависят от ответа сервиса и соединения. Изменения не включают работу над мобильной сборкой.

## Проверка

- Собраны Windows x64 / Qt 6.11.1 / LLVM MinGW и Linux / Qt 6.12 с включённым и выключенным WebEngine. Проверен автоматический выбор внешнего браузера в Windows-комплекте без WebEngine. Проверена конфигурация переносимого toolchain из репозитория.
- Windows упакован в `dist/windows`; PE-аудит DLL приложения, плагинов и QML оставляет внешними только системные библиотеки Windows. Архив — `dist/nameless-aurora-windows-x64.zip`.
- Оба Linux-варианта проходят smoke-запуск без QML-предупреждений. Проверен SQL-снимок: семь таблиц, версия 2, внешние ключи, индексы и представление создаются в пустой SQLite.
- Целевые тесты кэша, сохранения метаданных новой очереди, сброса неудачной ссылки, перебора источников, отмены и общего срока ожидания проходят. Проверки web-скриптов и импорта cookies также проходят.
- Полный `ctest`: 74 успешных QtTest-проверки и два сбоя. `homeVkRecommendationsStaySeparateFromSearch` ждёт 9 секунд при фактическом ожидании около 12 секунд; такой же сбой воспроизведён на старом бинарнике до изменений. `automaticSourcesTryNextServiceAndAttachPlayableAudio` предполагает определённый порядок завершения параллельного поиска; отдельно он проходит. Оба набора тестов анимации сообщают четыре ошибки поиска объектов `bonsaiLeftRoot`/`bonsaiRightRoot` в существующей сцене; эти файлы в задаче не изменялись.
- Запуск Windows через Wine заблокирован средой: `wineserver: bind: Operation not permitted`. Реальная Windows, реальные аккаунты сервисов и скорость внешних потоков в этой среде не проверены.

Логи проверки находятся в `build/verify-ctest.log`, `build/online-targeted-tests.txt`, `build/automatic-sources-isolated.txt`, `build/baseline-targeted-tests.txt`, `build/linux-smoke.log`, `build/linux-webengine-smoke.log`, `build/windows-dependency-audit.json` и `build/windows-wine-smoke.log`.
