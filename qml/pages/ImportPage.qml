// Страница импорта: перенос из Яндекса/VK, локальные файлы и загрузка по ссылке или поиску YouTube.
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import "../common" as Common
import "../components" as Components

Item {
    id: root
    property bool compact: false
    property bool searchMode: false
    property var selected: ({})
    property int selectedCount: 0
    property bool pendingNative: false
    property bool pendingInspect: false
    // Список профилей браузера: первым пунктом — автоопределение, затем найденные локальные профили.
    property var localProfiles: [
        {
            id: "",
            label: "Автоматически · найти существующий вход"
        }
    ].concat(webLogin.localBrowserProfiles())
    Theme {
        id: theme
    }
    function quality() {
        return settingsStore.quality;
    }
    function profileId() {
        return localProfiles[localProfile.currentIndex]?.id || "";
    }
    function downloadable(track) {
        return !track.downloaded && downloadManager.serviceForUrl(track.url || "").length > 0;
    }
    function chooseAll(value) {
        let next = {}, count = 0;
        for (let i = 0; i < libraryExchange.importedTracks.length; i++) {
            const track = libraryExchange.importedTracks[i];
            if (value && downloadable(track)) {
                next[i] = true;
                count++;
            }
        }
        selected = next;
        selectedCount = count;
    }
    function toggle(index) {
        let next = Object.assign({}, selected);
        if (next[index]) {
            delete next[index];
            selectedCount--;
        } else {
            next[index] = true;
            selectedCount++;
        }
        selected = next;
    }
    // Разделяет выбранные треки на «нативные» (VK/Яндекс через браузер) и обычные ссылки, и ставит их в очередь.
    function startNative() {
        let tracks = [], urls = [], titles = [];
        for (let i = 0; i < libraryExchange.importedTracks.length; i++)
            if (selected[i]) {
                const track = libraryExchange.importedTracks[i], service = downloadManager.serviceForUrl(track.url);
                if (service === "VK" || service === "Яндекс Музыка")
                    tracks.push(track);
                else {
                    urls.push(track.url);
                    titles.push(track.artist + " — " + track.title);
                }
            }
        if (tracks.length)
            downloadManager.enqueueNative(tracks, quality());
        if (urls.length)
            downloadManager.enqueue(urls, quality(), false, titles);
        chooseAll(false);
    }
    // Определяет, куда вести запрос: в перенос из браузера, в поиск YouTube или в разбор ссылки.
    function inspect() {
        if (!audioUrl.text.trim().length) {
            toast.show("Вставь ссылку или название трека");
            return;
        }
        const service = downloadManager.serviceForUrl(audioUrl.text);
        if (!searchMode && (service === "VK" || service === "Яндекс Музыка")) {
            libraryExchange.importFromBrowser(audioUrl.text, profileId());
            return;
        }
        const available = !searchMode && service === "Spotify" ? downloadDependencies.spotdlAvailable : downloadDependencies.ytDlpAvailable;
        if (!available) {
            pendingInspect = true;
            downloadDependencies.installOrUpdate();
            return;
        }
        pendingInspect = false;
        if (searchMode)
            importPreviewer.searchYoutube(audioUrl.text, quality());
        else
            importPreviewer.inspect(audioUrl.text, quality(), false);
    }
    // Проверяет место и подтверждение перед массовой загрузкой, затем запускает её.
    function requestDownload(isNative) {
        pendingNative = isNative;
        const count = isNative ? selectedCount : importPreviewer.selectedCount;
        if (!count)
            return;
        if (!isNative && importPreviewer.selectedBytes > 0 && !storageManager.canFit(importPreviewer.selectedBytes)) {
            toast.show("Недостаточно места. Выбери меньше треков или другую папку.");
            return;
        }
        if (settingsStore.confirmBulkDownload && count > 1)
            confirmDialog.open();
        else if (isNative)
            startNative();
        else
            importPreviewer.startSelected();
    }
    Connections {
        target: libraryExchange
        function onCompleted(id) {
            root.chooseAll(true);
        }
        function onAudioImportRequested(tracks) {
            root.chooseAll(false);
        }
    }
    // После установки недостающих инструментов автоматически повторяем отложенный разбор.
    Connections {
        target: downloadDependencies
        function onBusyChanged() {
            if (root.pendingInspect && !downloadDependencies.busy)
                Qt.callLater(function () {
                    if (!root.pendingInspect || downloadDependencies.busy)
                        return;
                    root.pendingInspect = false;
                    const ready = !root.searchMode && downloadManager.serviceForUrl(audioUrl.text) === "Spotify" ? downloadDependencies.spotdlAvailable : downloadDependencies.ytDlpAvailable;
                    if (ready)
                        root.inspect();
                    else
                        toast.show(downloadDependencies.lastError || downloadDependencies.statusText);
                });
        }
        function onLastErrorChanged() {
            if (downloadDependencies.lastError.length) {
                root.pendingInspect = false;
                toast.show(downloadDependencies.lastError);
            }
        }
    }
    Connections {
        target: webLogin
        function onLoginDone(service, ok, message) {
            toast.show(message);
        }
    }
    ScrollView {
        id: scroll
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth
        ScrollBar.vertical: Common.AuroraScrollBar {
            uiTheme: theme
            x: scroll.width - width
            height: scroll.height
        }
        ColumnLayout {
            width: Math.min(920, scroll.availableWidth - (compact ? 32 : 64))
            x: (scroll.availableWidth - width) / 2
            spacing: 20
            Item {
                implicitHeight: compact ? 12 : 24
            }
            Text {
                text: "Собери свою музыку"
                color: theme.text
                font.pixelSize: compact ? 26 : 32
                font.weight: Font.DemiBold
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }
            Text {
                text: "Любимые плейлисты и локальные файлы — в одной коллекции."
                color: theme.subtext
                font.pixelSize: 14
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: connectContent.implicitHeight + 40
                color: theme.surface
                radius: 22
                border.color: theme.line
                ColumnLayout {
                    id: connectContent
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 20
                    spacing: 16
                    Text {
                        text: "Из Яндекса и VK"
                        color: theme.text
                        font.pixelSize: 22
                        font.weight: Font.DemiBold
                    }
                    Text {
                        Layout.fillWidth: true
                        text: "Aurora использует сохранённый вход выбранного браузера, переносит список музыки и ставит аудио на загрузку. Если сервис не отдаст файл, в панели загрузок можно выбрать другой источник."
                        color: theme.subtext
                        font.pixelSize: 13
                        wrapMode: Text.WordWrap
                        lineHeight: 1.4
                    }
                    Common.AuroraCombo {
                        id: localProfile
                        uiTheme: theme
                        Layout.fillWidth: true
                        model: root.localProfiles
                        textRole: "label"
                        enabled: !libraryExchange.busy && !webLogin.busy
                    }
                    Flow {
                        Layout.fillWidth: true
                        spacing: 8
                        Repeater {
                            model: ["Яндекс Музыка", "VK"]
                            delegate: Common.AuroraButton {
                                required property string modelData
                                uiTheme: theme
                                text: "Забрать из " + (modelData === "VK" ? "VK" : "Яндекса")
                                iconName: "download"
                                primary: true
                                enabled: !webLogin.busy && !libraryExchange.busy
                                onClicked: libraryExchange.importMyMusic(modelData, root.profileId())
                            }
                        }
                    }
                    Common.AuroraField {
                        id: playlistUrl
                        uiTheme: theme
                        Layout.fillWidth: true
                        iconName: "link"
                        placeholderText: "Или ссылка на отдельный плейлист / альбом"
                        onAccepted: libraryExchange.importFromBrowser(text, root.profileId())
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Common.AuroraButton {
                            uiTheme: theme
                            Layout.fillWidth: true
                            text: libraryExchange.busy ? "Переносим музыку…" : "Перенести плейлист в плеер"
                            primary: true
                            iconName: "download"
                            enabled: !libraryExchange.busy && !webLogin.busy && playlistUrl.text.trim().length > 0
                            onClicked: libraryExchange.importFromBrowser(playlistUrl.text, root.profileId())
                        }
                        Common.AuroraButton {
                            uiTheme: theme
                            text: "Отмена"
                            visible: libraryExchange.busy
                            onClicked: libraryExchange.cancel()
                        }
                    }
                    Common.AuroraProgress {
                        uiTheme: theme
                        Layout.fillWidth: true
                        visible: libraryExchange.busy
                        indeterminate: true
                    }
                    Text {
                        Layout.fillWidth: true
                        visible: text.length > 0
                        text: libraryExchange.busy ? webLogin.status : libraryExchange.status
                        color: theme.accent
                        font.pixelSize: 12
                        wrapMode: Text.WordWrap
                    }
                    Common.AuroraButton {
                        uiTheme: theme
                        text: "Открыть отчёт импорта"
                        visible: webLogin.importReport.length > 0
                        implicitHeight: 32
                        onClicked: webLogin.openImportReport()
                    }
                    Text {
                        Layout.fillWidth: true
                        text: "Сохраняется аудио, доступное твоему аккаунту. Если сервис отдаёт только фрагмент, Aurora сообщит об этом."
                        color: theme.textMuted
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                    }
                    Common.AuroraButton {
                        uiTheme: theme
                        text: extras.visible ? "Скрыть дополнительные способы" : "Отдельный вход / сбросить сессию Aurora"
                        implicitHeight: 32
                        onClicked: extras.visible = !extras.visible
                    }
                    ColumnLayout {
                        id: extras
                        visible: false
                        Layout.fillWidth: true
                        spacing: 8
                        Text {
                            Layout.fillWidth: true
                            text: "Запасной вариант, если сохранённый вход недоступен. Эти кнопки открывают отдельное окно; основной перенос выше использует существующий профиль без окна."
                            color: theme.subtext
                            font.pixelSize: 12
                            wrapMode: Text.WordWrap
                        }
                        Common.AuroraCombo {
                            id: externalBrowser
                            uiTheme: theme
                            Layout.fillWidth: true
                            model: webLogin.availableBrowsers()
                        }
                        Flow {
                            Layout.fillWidth: true
                            spacing: 8
                            Common.AuroraButton {
                                uiTheme: theme
                                text: "Яндекс · Внешний вход"
                                enabled: !webLogin.busy && externalBrowser.count > 0
                                onClicked: webLogin.startLogin("Яндекс Музыка", externalBrowser.currentIndex)
                            }
                            Common.AuroraButton {
                                uiTheme: theme
                                text: "VK · Внешний вход"
                                enabled: !webLogin.busy && externalBrowser.count > 0
                                onClicked: webLogin.startLogin("VK", externalBrowser.currentIndex)
                            }
                            Common.AuroraButton {
                                uiTheme: theme
                                text: "Отключить Яндекс"
                                enabled: webLogin.yandexLoggedIn && !webLogin.busy
                                onClicked: {
                                    webLogin.logout("yandex");
                                    accounts.logout("Яндекс Музыка");
                                }
                            }
                            Common.AuroraButton {
                                uiTheme: theme
                                text: "Отключить VK"
                                enabled: webLogin.vkLoggedIn && !webLogin.busy
                                onClicked: {
                                    webLogin.logout("vk");
                                    accounts.logout("VK");
                                }
                            }
                        }
                        Common.AuroraButton {
                            uiTheme: theme
                            text: "Прочитать ссылку через отдельный вход"
                            enabled: !webLogin.busy && !libraryExchange.busy && playlistUrl.text.trim().length > 0
                            onClicked: libraryExchange.importUrl(playlistUrl.text)
                        }
                        Text {
                            Layout.fillWidth: true
                            text: webLogin.status
                            color: theme.subtext
                            wrapMode: Text.WordWrap
                            font.pixelSize: 12
                        }
                        Common.AuroraButton {
                            uiTheme: theme
                            text: "Отменить вход"
                            visible: webLogin.busy
                            onClicked: webLogin.cancel()
                        }
                    }
                }
            }
            Rectangle {
                visible: libraryExchange.importedTracks.length > 0
                Layout.fillWidth: true
                implicitHeight: nativeContent.implicitHeight + 32
                color: theme.surface
                radius: 20
                border.color: theme.line
                ColumnLayout {
                    id: nativeContent
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 16
                    spacing: 12
                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            Layout.fillWidth: true
                            text: root.selectedCount + " / " + libraryExchange.importedTracks.length + " выбрано"
                            color: theme.text
                            font.pixelSize: 16
                            font.weight: Font.Medium
                        }
                        Common.AuroraButton {
                            uiTheme: theme
                            text: root.selectedCount ? "Снять выбор" : "Выбрать все"
                            implicitHeight: 36
                            onClicked: root.chooseAll(root.selectedCount === 0)
                        }
                    }
                    ListView {
                        id: nativeList
                        Layout.fillWidth: true
                        implicitHeight: Math.min(360, contentHeight)
                        clip: true
                        model: libraryExchange.importedTracks
                        boundsBehavior: Flickable.StopAtBounds
                        delegate: Item {
                            required property var modelData
                            required property int index
                            width: nativeList.width
                            height: 58
                            RowLayout {
                                anchors.fill: parent
                                spacing: 12
                                CheckBox {
                                    checked: !!root.selected[index]
                                    enabled: root.downloadable(modelData)
                                    onClicked: root.toggle(index)
                                    palette.highlight: theme.accent
                                    palette.window: theme.surfaceHigh
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 4
                                    Text {
                                        Layout.fillWidth: true
                                        text: modelData.title
                                        color: theme.text
                                        font.pixelSize: 13
                                        elide: Text.ElideRight
                                    }
                                    Text {
                                        Layout.fillWidth: true
                                        text: modelData.artist || "Неизвестный исполнитель"
                                        color: theme.subtext
                                        font.pixelSize: 11
                                        elide: Text.ElideRight
                                    }
                                }
                                Text {
                                    text: modelData.downloaded ? "В плеере" : modelData.durationMs > 0 ? Math.floor(modelData.durationMs / 60000) + ":" + String(Math.floor(modelData.durationMs / 1000) % 60).padStart(2, "0") : ""
                                    color: theme.subtext
                                    font.pixelSize: 11
                                }
                            }
                        }
                    }
                    Common.AuroraSegmented {
                        uiTheme: theme
                        Layout.fillWidth: true
                        model: ["Оригинал", "192 kbps", "128 kbps"]
                        currentIndex: ["original", "balanced", "compact"].indexOf(settingsStore.quality)
                        onOptionSelected: index => settingsStore.quality = ["original", "balanced", "compact"][index]
                    }
                    Common.AuroraButton {
                        uiTheme: theme
                        Layout.fillWidth: true
                        primary: true
                        text: "Скачать в плеер · " + root.selectedCount
                        iconName: "download"
                        enabled: root.selectedCount > 0
                        onClicked: root.requestDownload(true)
                    }
                }
            }
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: fileContent.implicitHeight + 32
                color: theme.surface
                radius: 20
                border.color: theme.line
                ColumnLayout {
                    id: fileContent
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 16
                    spacing: 12
                    Text {
                        text: "С устройства"
                        color: theme.text
                        font.pixelSize: 18
                        font.weight: Font.Medium
                    }
                    Flow {
                        Layout.fillWidth: true
                        spacing: 8
                        Common.AuroraButton {
                            uiTheme: theme
                            text: "Аудиофайлы"
                            iconName: "plus"
                            onClicked: audioFiles.open()
                        }
                        Common.AuroraButton {
                            uiTheme: theme
                            text: "TXT / CSV / JSON"
                            iconName: "folder"
                            enabled: !libraryExchange.busy
                            onClicked: listFile.open()
                        }
                        Common.AuroraButton {
                            uiTheme: theme
                            text: "Медиатека Aurora"
                            iconName: "library"
                            enabled: !libraryExchange.busy
                            onClicked: bundleFile.open()
                        }
                    }
                }
            }
            Text {
                text: "YouTube и другие ссылки"
                color: theme.text
                font.pixelSize: 22
                font.weight: Font.DemiBold
            }
            Common.AuroraSegmented {
                uiTheme: theme
                Layout.fillWidth: true
                model: ["По ссылке", "Поиск YouTube"]
                onOptionSelected: index => root.searchMode = index === 1
            }
            Common.AuroraField {
                id: audioUrl
                uiTheme: theme
                Layout.fillWidth: true
                iconName: searchMode ? "search" : "link"
                placeholderText: searchMode ? "Исполнитель или название" : "Ссылка на трек или плейлист"
                onAccepted: root.inspect()
            }
            Common.AuroraSegmented {
                uiTheme: theme
                Layout.fillWidth: true
                model: ["Оригинал", "192 kbps", "128 kbps"]
                currentIndex: ["original", "balanced", "compact"].indexOf(settingsStore.quality)
                onOptionSelected: index => settingsStore.quality = ["original", "balanced", "compact"][index]
            }
            Common.AuroraButton {
                uiTheme: theme
                Layout.fillWidth: true
                primary: true
                text: searchMode ? "Найти музыку" : "Показать содержимое"
                iconName: "search"
                enabled: !importPreviewer.busy && !downloadDependencies.busy && !libraryExchange.busy
                onClicked: root.inspect()
            }
            Common.AuroraProgress {
                uiTheme: theme
                Layout.fillWidth: true
                visible: importPreviewer.busy || downloadDependencies.busy
                indeterminate: true
            }
            Common.AuroraButton {
                uiTheme: theme
                text: "Отменить получение списка"
                visible: importPreviewer.busy
                onClicked: importPreviewer.cancel()
            }
            Text {
                Layout.fillWidth: true
                text: importPreviewer.lastError || (downloadDependencies.busy ? downloadDependencies.statusText : importPreviewer.statusText)
                visible: text.length > 0
                color: importPreviewer.lastError ? theme.danger : theme.subtext
                font.pixelSize: 12
                wrapMode: Text.WordWrap
            }
            ListView {
                id: previewList
                Layout.fillWidth: true
                implicitHeight: Math.min(340, contentHeight)
                visible: count > 0
                model: importPreviewer
                clip: true
                delegate: Item {
                    required property int index
                    required property string title
                    required property string artist
                    required property bool selected
                    width: previewList.width
                    height: 58
                    RowLayout {
                        anchors.fill: parent
                        CheckBox {
                            checked: selected
                            onClicked: importPreviewer.toggle(index)
                            palette.highlight: theme.accent
                        }
                        ColumnLayout {
                            Layout.fillWidth: true
                            Text {
                                text: title
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                                color: theme.text
                                font.pixelSize: 13
                            }
                            Text {
                                text: artist
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                                color: theme.subtext
                                font.pixelSize: 11
                            }
                        }
                    }
                }
            }
            Flow {
                Layout.fillWidth: true
                spacing: 8
                visible: previewList.count > 0
                Common.AuroraButton {
                    uiTheme: theme
                    text: "Выбрать все"
                    onClicked: importPreviewer.selectAll(true)
                }
                Common.AuroraButton {
                    uiTheme: theme
                    primary: true
                    text: "Скачать выбранное · " + importPreviewer.selectedCount
                    enabled: importPreviewer.selectedCount > 0
                    onClicked: root.requestDownload(false)
                }
            }
            Components.DependencyCard {
                Layout.fillWidth: true
            }
            Components.StorageSummary {
                Layout.fillWidth: true
            }
            Text {
                Layout.fillWidth: true
                text: "Прогресс и ошибки доступны в панели загрузок над плеером на любом экране."
                color: theme.textMuted
                font.pixelSize: 12
                wrapMode: Text.WordWrap
                visible: downloadManager.totalCount > 0
            }
            Item {
                implicitHeight: 24
            }
        }
    }
    FileDialog {
        id: audioFiles
        fileMode: FileDialog.OpenFiles
        nameFilters: ["Музыка (*.mp3 *.flac *.wav *.ogg *.opus *.m4a *.aac *.webm *.mka)"]
        onAccepted: importManager.importFiles(selectedFiles)
    }
    FileDialog {
        id: listFile
        nameFilters: ["Плейлист (*.txt *.csv *.json)"]
        onAccepted: libraryExchange.importList(selectedFile, "Другой сервис")
    }
    FileDialog {
        id: bundleFile
        nameFilters: ["Медиатека Aurora (aurora-library.json)"]
        onAccepted: libraryExchange.importList(selectedFile, "Aurora")
    }
    Common.AuroraDialog {
        id: confirmDialog
        uiTheme: theme
        title: "Скачать в плеер?"
        width: Math.min(root.width - 32, 440)
        contentItem: ColumnLayout {
            spacing: 18
            Text {
                Layout.fillWidth: true
                text: "Выбрано треков: " + (root.pendingNative ? root.selectedCount : importPreviewer.selectedCount) + ". Файлы сохранятся в папке музыки и появятся в библиотеке."
                color: theme.text
                wrapMode: Text.WordWrap
                font.pixelSize: 14
            }
            RowLayout {
                Layout.fillWidth: true
                Common.AuroraButton {
                    uiTheme: theme
                    Layout.fillWidth: true
                    text: "Отмена"
                    onClicked: confirmDialog.close()
                }
                Common.AuroraButton {
                    uiTheme: theme
                    Layout.fillWidth: true
                    primary: true
                    text: "Скачать"
                    onClicked: {
                        confirmDialog.close();
                        if (root.pendingNative)
                            root.startNative();
                        else
                            importPreviewer.startSelected();
                    }
                }
            }
        }
    }
}
