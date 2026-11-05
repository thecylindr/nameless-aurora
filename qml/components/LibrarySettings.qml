// Настройки библиотеки: папка, качество загрузки, источники автоподбора, браузер и компоненты загрузки.
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import "../common" as Common

ColumnLayout {
    id: root
    property bool compact: false
    property bool browserExpanded: false
    property bool toolsExpanded: false
    property bool manualBrowserChosen: false
    readonly property bool narrow: width < 580
    readonly property var detectedBrowsers: browserAuth.detectedBrowsers()
    readonly property bool manualBrowserEntry: manualBrowserChosen || detectedBrowsers.indexOf(settingsStore.browser) < 0
    // Доли занятого места и музыки на диске для индикатора заполненности.
    readonly property real usedRatio: Math.max(0, Math.min(1, (storageManager.totalBytes - storageManager.freeBytes) / Math.max(1, storageManager.totalBytes)))
    readonly property real musicRatio: Math.max(0, Math.min(usedRatio, storageManager.libraryBytes / Math.max(1, storageManager.totalBytes)))
    spacing: 20
    Theme {
        id: theme
    }
    // Преобразует путь в file://-ссылку, кодируя каждую часть, чтобы корректно открывать имена с пробелами и кириллицей.
    function fileUrl(path) {
        return "file://" + path.split('/').map(encodeURIComponent).join('/');
    }
    // При первом показе подбираем подходящий браузер и профиль, если они ещё не заданы.
    Component.onCompleted: {
        storageManager.refresh();
        if (detectedBrowsers.length && detectedBrowsers.indexOf(settingsStore.browser) < 0)
            settingsStore.browser = browserAuth.suggestBrowser();
        if (!settingsStore.browserProfile.length)
            settingsStore.browserProfile = browserAuth.suggestFirefoxProfile();
    }
    Rectangle {
        Layout.fillWidth: true
        implicitHeight: overview.implicitHeight + 40
        radius: 24
        color: theme.surface
        border.color: theme.line
        ColumnLayout {
            id: overview
            anchors.fill: parent
            anchors.margins: 20
            spacing: 18
            RowLayout {
                Layout.fillWidth: true
                spacing: 12
                Rectangle {
                    Layout.preferredWidth: 44
                    Layout.preferredHeight: 44
                    radius: 14
                    color: theme.selected
                    Common.AuroraIcon {
                        anchors.centerIn: parent
                        name: "library"
                        size: 22
                        color: theme.accent
                    }
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    Text {
                        text: "Твоя коллекция"
                        color: theme.text
                        font.pixelSize: 22
                        font.weight: Font.DemiBold
                    }
                    Text {
                        Layout.fillWidth: true
                        text: "Музыка, обложки и плейлисты на этом устройстве"
                        color: theme.subtext
                        font.pixelSize: 12
                        wrapMode: Text.WordWrap
                    }
                }
            }
            GridLayout {
                Layout.fillWidth: true
                columns: root.narrow ? 2 : 4
                columnSpacing: 8
                rowSpacing: 8
                Repeater {
                    model: [
                        {
                            value: String(libraryModel.totalCount),
                            label: "в коллекции"
                        },
                        {
                            value: String(libraryModel.downloadedCount),
                            label: "скачано"
                        },
                        {
                            value: String(libraryModel.missingCount),
                            label: "ещё без файла"
                        },
                        {
                            value: String(libraryModel.playlists.length),
                            label: "плейлистов"
                        }
                    ]
                    Rectangle {
                        required property var modelData
                        Layout.fillWidth: true
                        implicitHeight: root.narrow ? 82 : 92
                        radius: 15
                        color: theme.surfaceHigh
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 5
                            Text {
                                Layout.fillWidth: true
                                text: modelData.value
                                color: theme.text
                                font.pixelSize: root.narrow ? 20 : 26
                                font.weight: Font.DemiBold
                                elide: Text.ElideRight
                            }
                            Text {
                                text: modelData.label
                                color: theme.subtext
                                font.pixelSize: 11
                            }
                        }
                    }
                }
            }
            Text {
                Layout.fillWidth: true
                text: (storageManager.busy ? "Подсчитываем размер…" : storageManager.formatBytes(storageManager.libraryBytes) + " · музыка на устройстве") + ". Скачанные треки доступны без интернета."
                color: theme.good
                font.pixelSize: 12
                wrapMode: Text.WordWrap
            }
            Common.AuroraButton {
                uiTheme: theme
                text: "Исправить названия VK"
                iconName: "refresh"
                enabled: !libraryExchange.busy
                onClicked: libraryExchange.repairVkMetadata()
            }
            Text {
                Layout.fillWidth: true
                visible: libraryExchange.busy || libraryExchange.status.length > 0
                text: libraryExchange.status
                color: theme.subtext
                font.pixelSize: 11
                wrapMode: Text.WordWrap
                maximumLineCount: 3
                elide: Text.ElideRight
            }
            RowLayout {
                Layout.fillWidth: true
                Text {
                    text: "Место на диске"
                    color: theme.subtext
                    font.pixelSize: 12
                    Layout.fillWidth: true
                }
                Common.AuroraButton {
                    uiTheme: theme
                    iconName: "refresh"
                    iconOnly: true
                    text: "Обновить размер библиотеки"
                    implicitWidth: 28
                    implicitHeight: 28
                    enabled: !storageManager.busy
                    onClicked: {
                        storageManager.refresh();
                        libraryModel.refresh();
                    }
                }
            }
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 8
                radius: 4
                color: theme.surfaceHigh
                clip: true
                // Общий объём занятого места, поверх него — более яркая полоса музыки Aurora.
                Rectangle {
                    width: parent.width * root.usedRatio
                    height: parent.height
                    radius: 4
                    color: Qt.rgba(theme.accent.r, theme.accent.g, theme.accent.b, .3)
                }
                Rectangle {
                    width: Math.max(storageManager.libraryBytes > 0 ? 3 : 0, parent.width * root.musicRatio)
                    height: parent.height
                    radius: 4
                    color: theme.accent
                }
            }
            Text {
                Layout.fillWidth: true
                text: storageManager.busy ? "Подсчитываем файлы…" : storageManager.formatBytes(storageManager.freeBytes) + " свободно из " + storageManager.formatBytes(storageManager.totalBytes)
                color: theme.subtext
                font.pixelSize: 12
                wrapMode: Text.WordWrap
            }
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 1
                color: theme.line
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: 10
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 5
                    Text {
                        text: "Папка музыки"
                        color: theme.text
                        font.pixelSize: 13
                        font.weight: Font.Medium
                    }
                    Text {
                        Layout.fillWidth: true
                        text: settingsStore.libraryFolder
                        color: theme.subtext
                        font.pixelSize: 12
                        elide: Text.ElideMiddle
                    }
                }
                Common.AuroraButton {
                    uiTheme: theme
                    text: "Изменить"
                    iconName: "folder"
                    iconOnly: root.narrow
                    implicitHeight: 36
                    onClicked: folder.open()
                }
            }
            Text {
                Layout.fillWidth: true
                text: "Новые файлы сохраняются сюда. Смена папки не перемещает уже добавленную музыку."
                color: theme.textMuted
                font.pixelSize: 11
                wrapMode: Text.WordWrap
            }
            Flow {
                Layout.fillWidth: true
                spacing: 8
                Common.AuroraButton {
                    uiTheme: theme
                    text: "Добавить файлы"
                    iconName: "plus"
                    implicitHeight: 36
                    onClicked: audioFiles.open()
                }
                Common.AuroraButton {
                    uiTheme: theme
                    text: "Открыть папку"
                    iconName: "folder"
                    implicitHeight: 36
                    onClicked: Qt.openUrlExternally(root.fileUrl(settingsStore.libraryFolder))
                }
                Common.AuroraButton {
                    uiTheme: theme
                    text: "Копия с музыкой"
                    iconName: "upload"
                    implicitHeight: 36
                    enabled: !libraryExchange.busy && libraryModel.totalCount > 0
                    onClicked: backup.open()
                }
            }
        }
    }
    Common.AuroraSection {
        uiTheme: theme
        Layout.fillWidth: true
        title: "Как сохранять музыку"
        description: "Качество, очередь и поведение при ошибке"
        content: ColumnLayout {
            width: parent.width
            spacing: 16
            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                Repeater {
                    model: [
                        {
                            id: "original",
                            name: "Оригинал",
                            description: "Как в источнике"
                        },
                        {
                            id: "balanced",
                            name: "MP3 · 192",
                            description: "Баланс размера"
                        },
                        {
                            id: "compact",
                            name: "MP3 · 128",
                            description: "Меньше места"
                        }
                    ]
                    Rectangle {
                        id: qualityCard
                        required property var modelData
                        Layout.fillWidth: true
                        implicitHeight: 86
                        radius: 15
                        color: settingsStore.quality === modelData.id ? theme.selected : theme.surface
                        border.color: settingsStore.quality === modelData.id ? theme.accent : theme.line
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 6
                            Text {
                                Layout.fillWidth: true
                                text: qualityCard.modelData.name
                                color: theme.text
                                font.pixelSize: root.narrow ? 12 : 14
                                font.weight: Font.Medium
                                elide: Text.ElideRight
                            }
                            Text {
                                Layout.fillWidth: true
                                text: qualityCard.modelData.description
                                color: theme.subtext
                                font.pixelSize: 10
                                wrapMode: Text.WordWrap
                            }
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: settingsStore.quality = qualityCard.modelData.id
                        }
                        activeFocusOnTab: true
                        Accessible.role: Accessible.RadioButton
                        Accessible.name: modelData.name
                        Accessible.checked: settingsStore.quality === modelData.id
                        Keys.onSpacePressed: settingsStore.quality = modelData.id
                        Keys.onReturnPressed: settingsStore.quality = modelData.id
                        Accessible.onPressAction: settingsStore.quality = modelData.id
                    }
                }
            }
            Text {
                Layout.fillWidth: true
                text: "Оригинал сохраняет формат доступного файла; MP3 преобразуется с выбранным битрейтом."
                color: theme.textMuted
                font.pixelSize: 11
                wrapMode: Text.WordWrap
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: 12
                Text {
                    Layout.fillWidth: true
                    text: "Одновременно загружать"
                    color: theme.text
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                }
                Common.AuroraCombo {
                    uiTheme: theme
                    Layout.preferredWidth: root.narrow ? 120 : 200
                    model: ["1 трек", "2 трека", "3 трека", "4 трека"]
                    currentIndex: settingsStore.downloadConcurrency - 1
                    onActivated: settingsStore.downloadConcurrency = currentIndex + 1
                }
            }
            Common.AuroraSwitch {
                uiTheme: theme
                Layout.fillWidth: true
                text: "Подтверждать массовую загрузку"
                subtext: "Показать число треков и оценку размера до старта"
                checked: settingsStore.confirmBulkDownload
                onToggled: settingsStore.confirmBulkDownload = checked
            }
            Common.AuroraSwitch {
                uiTheme: theme
                Layout.fillWidth: true
                text: "Подбирать аудио при ошибке"
                subtext: "Перебирать включённые источники и подходящие версии"
                checked: audioSources.automaticRecovery
                onToggled: audioSources.automaticRecovery = checked
            }
            Text {
                Layout.fillWidth: true
                text: "Источники автоподбора"
                color: theme.text
                font.pixelSize: 14
                font.weight: Font.Medium
            }
            Text {
                Layout.fillWidth: true
                text: "После отказа пробуем до трёх подходящих записей, затем следующий сервис. Проверка названия и версии сохраняется."
                color: theme.subtext
                font.pixelSize: 11
                wrapMode: Text.WordWrap
            }
            Common.AuroraCombo {
                uiTheme: theme
                Layout.fillWidth: true
                model: audioSources.recoverySources.length ? audioSources.recoverySources : ["Нет источников"]
                onActivated: if (audioSources.recoverySources.length) {
                    const first = currentText;
                    audioSources.recoverySources = [first].concat(audioSources.recoverySources.filter(source => source !== first));
                }
            }
            Repeater {
                model: audioSources.sources
                delegate: Common.AuroraSwitch {
                    required property string modelData
                    uiTheme: theme
                    Layout.fillWidth: true
                    text: modelData
                    enabled: audioSources.automaticRecovery
                    checked: audioSources.recoverySources.indexOf(modelData) >= 0
                    onToggled: {
                        let order = audioSources.recoverySources.slice();
                        const i = order.indexOf(modelData);
                        if (checked && i < 0)
                            order.push(modelData);
                        else if (!checked && i >= 0)
                            order.splice(i, 1);
                        audioSources.recoverySources = order;
                    }
                }
            }
        }
    }
    Rectangle {
        Layout.fillWidth: true
        implicitHeight: browserBody.implicitHeight + 32
        radius: 20
        color: theme.surface
        border.color: theme.line
        ColumnLayout {
            id: browserBody
            anchors.fill: parent
            anchors.margins: 16
            spacing: 14
            RowLayout {
                Layout.fillWidth: true
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 5
                    Text {
                        text: "Браузер и вход в сервисы"
                        color: theme.text
                        font.pixelSize: 16
                        font.weight: Font.Medium
                    }
                    Text {
                        Layout.fillWidth: true
                        text: settingsStore.browser + " · сохранённые сессии"
                        color: theme.subtext
                        font.pixelSize: 12
                        elide: Text.ElideRight
                    }
                }
                Common.AuroraButton {
                    uiTheme: theme
                    iconName: "settings"
                    text: root.browserExpanded ? "Скрыть" : "Настроить"
                    iconOnly: root.narrow
                    implicitHeight: 34
                    onClicked: root.browserExpanded = !root.browserExpanded
                }
            }
            ColumnLayout {
                visible: root.browserExpanded
                Layout.fillWidth: true
                spacing: 12
                Text {
                    Layout.fillWidth: true
                    text: "Профиль для VK и Яндекса выбирается на странице импорта. Здесь — браузер и профиль для загрузчиков."
                    color: theme.subtext
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                }
                Common.AuroraCombo {
                    uiTheme: theme
                    Layout.fillWidth: true
                    model: root.detectedBrowsers.concat(["Вручную…"])
                    currentIndex: {
                        const i = root.detectedBrowsers.indexOf(settingsStore.browser);
                        return i >= 0 ? i : root.detectedBrowsers.length;
                    }
                    onActivated: index => {
                        root.manualBrowserChosen = index >= root.detectedBrowsers.length;
                        if (!root.manualBrowserChosen)
                            settingsStore.browser = root.detectedBrowsers[index];
                    }
                }
                Common.AuroraField {
                    uiTheme: theme
                    Layout.fillWidth: true
                    visible: root.manualBrowserEntry
                    placeholderText: "Имя браузера"
                    text: settingsStore.browser
                    onEditingFinished: settingsStore.browser = text
                }
                Common.AuroraField {
                    uiTheme: theme
                    Layout.fillWidth: true
                    placeholderText: "Профиль браузера · необязательно"
                    text: settingsStore.browserProfile
                    onEditingFinished: settingsStore.browserProfile = text
                }
                Text {
                    Layout.fillWidth: true
                    text: "Используется сохранённый вход выбранного музыкального сервиса. Личный профиль браузера остаётся на месте."
                    color: theme.textMuted
                    font.pixelSize: 11
                    wrapMode: Text.WordWrap
                }
            }
        }
    }
    Rectangle {
        Layout.fillWidth: true
        implicitHeight: toolsBody.implicitHeight + 32
        radius: 20
        color: theme.surface
        border.color: theme.line
        ColumnLayout {
            id: toolsBody
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12
            RowLayout {
                Layout.fillWidth: true
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 5
                    Text {
                        text: "Компоненты загрузки"
                        color: theme.text
                        font.pixelSize: 16
                        font.weight: Font.Medium
                    }
                    Text {
                        Layout.fillWidth: true
                        text: downloadDependencies.busy ? downloadDependencies.statusText : downloadDependencies.runtimeReady ? "Готовы к работе" : "Подготовятся при первой загрузке"
                        color: downloadDependencies.runtimeReady ? theme.good : theme.subtext
                        font.pixelSize: 12
                        elide: Text.ElideRight
                    }
                }
                Common.AuroraButton {
                    uiTheme: theme
                    text: root.toolsExpanded ? "Скрыть" : "Подробнее"
                    iconName: "download"
                    iconOnly: root.narrow
                    implicitHeight: 34
                    onClicked: root.toolsExpanded = !root.toolsExpanded
                }
            }
            Loader {
                Layout.fillWidth: true
                Layout.preferredHeight: item ? item.implicitHeight : 0
                active: root.toolsExpanded
                sourceComponent: DependencyCard {}
            }
        }
    }
    Common.AuroraButton {
        uiTheme: theme
        text: "Вернуть параметры загрузки"
        iconName: "refresh"
        onClicked: {
            settingsStore.quality = "balanced";
            settingsStore.downloadConcurrency = 1;
            settingsStore.confirmBulkDownload = true;
            audioSources.automaticRecovery = true;
            audioSources.recoverySources = ["YouTube", "SoundCloud", "Zaycev.net", "Hitmo", "Internet Archive"];
        }
    }
    Text {
        Layout.fillWidth: true
        text: "Метаданные, обложки и состав плейлистов сохраняются автоматически. «Копия с музыкой» создаёт переносимую медиатеку с доступными аудиофайлами."
        color: theme.textMuted
        font.pixelSize: 11
        wrapMode: Text.WordWrap
        lineHeight: 1.4
    }
    FolderDialog {
        id: folder
        title: "Папка музыки"
        currentFolder: root.fileUrl(settingsStore.libraryFolder)
        onAccepted: settingsStore.libraryFolder = selectedFolder
    }
    FolderDialog {
        id: backup
        title: "Куда сохранить копию медиатеки"
        onAccepted: libraryExchange.exportPortable(selectedFolder, 0)
    }
    FileDialog {
        id: audioFiles
        title: "Добавить музыку"
        fileMode: FileDialog.OpenFiles
        nameFilters: ["Аудио (*.mp3 *.flac *.wav *.ogg *.opus *.m4a *.aac *.wma *.aiff)"]
        onAccepted: importManager.importFiles(selectedFiles)
    }
}
