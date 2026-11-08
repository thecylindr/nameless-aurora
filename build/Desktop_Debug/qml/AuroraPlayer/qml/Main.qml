// Главное окно приложения: навигация, страницы, панели воспроизведения и диалоги.
import QtQuick
import QtQuick.Dialogs
import QtQuick.Controls
import QtQuick.Layouts
import "common" as Common
import "components" as Components
import "pages" as Pages
import "web" as Web

ApplicationWindow {
    id: root
    visible: true
    width: 1440
    height: 900
    minimumWidth: 360
    minimumHeight: 560
    title: "Aurora Player"
    color: theme.background

    // Индекс текущей страницы (0 — Главная, 1 — Библиотека и т.д.).
    property int currentPage: 0
    // Режим «сейчас играет», разворачивающий полноэкранную панель.
    property bool listening: false
    // Последнее показанное уведомление обмена (чтобы не дублировать).
    property string lastExchangeNotice: ""
    // Активен ли ввод текста — используется для блокировки горячих клавиш.
    readonly property bool typing: activeFocusItem instanceof TextInput || activeFocusItem instanceof TextEdit
    font.family: Qt.fontFamilies().indexOf("Inter") >= 0 ? "Inter" : "Noto Sans"
    // Выполняет действие по строковому коду (используется внешними тестами/голосом).
    function exerciseAction(action) {
        if (action === "home-settings") {
            root.currentPage = 0;
            homePage.openPreferences();
            return true;
        }
        if (action === "home-providers") {
            root.currentPage = 0;
            homePage.showProviders();
            return true;
        }
        if (action === "home-track") {
            root.currentPage = 0;
            for (const mix of homePage.mixes) {
                const track = mix.tracks.find(item => item.downloaded && item.trackId !== player.currentTrackId);
                if (track) {
                    homeFeed.playTrack(track.trackId);
                    return player.currentTrackId === track.trackId;
                }
            }
            return false;
        }
        if (action.indexOf("home-period:") === 0) {
            root.currentPage = 0;
            homePage.period = Number(action.slice(12));
            return homePage.mixes.length > 0;
        }
        if (action === "home-mix") {
            root.currentPage = 0;
            if (!homePage.mixes.length)
                return false;
            homePage.openMix(homePage.mixes[0]);
            return true;
        }
        if (action.indexOf("wide-") === 0) {
            root.width = 2048;
            root.height = 1100;
            action = action.slice(5);
        }
        if (action === "library") {
            root.currentPage = 1;
            return true;
        }
        if (action === "equalizer") {
            equalizerDialog.open();
            return true;
        }
        if (action === "equalizer-on") {
            player.equalizer.selectPreset("Тёплый бас");
            player.equalizer.enabled = true;
            equalizerDialog.open();
            return true;
        }
        if (action === "settings-sound") {
            root.currentPage = 3;
            settingsPage.category = 1;
            return true;
        }
        if (action.indexOf("appearance:") === 0) {
            const parts = action.split(":");
            settingsStore.colorScheme = parts[1];
            settingsStore.sphereStyle = parts[2] || "ribbons";
            return true;
        }
        if (action.indexOf("import-list:") === 0) {
            root.currentPage = 2;
            libraryExchange.importList(action.slice(12), "VK");
            return libraryExchange.importedTracks.length > 0;
        }
        if (action === "native-login") {
            webLogin.openNative("VK");
            console.info("Native login attempt: " + webLogin.status);
            return webLogin.browserVisible || webLogin.status.length > 0;
        }
        if (action === "new-playlist") {
            createPlaylist();
            return playlistDialog.visible;
        }
        if (action === "track-menu")
            return libraryPage.openTrackActions();
        if (action === "source-dialog" || action === "source-results") {
            const tracks = libraryModel.allTrackDetails();
            if (!tracks.length)
                return false;
            audioSources.openTrack(tracks[0].trackId);
            if (action === "source-results")
                audioSources.search("YouTube", tracks[0].artist + " " + tracks[0].title);
            return true;
        }
        if (action === "edit-track") {
            const tracks = libraryModel.allTrackDetails();
            if (!tracks.length)
                return false;
            libraryModel.requestEdit(tracks[0].trackId);
            return true;
        }
        if (action === "edit-album") {
            libraryModel.requestAlbumEdit("");
            return true;
        }
        if (action === "downloads") {
            const tracks = libraryModel.allTrackDetails().filter(track => !track.downloaded);
            if (!tracks.length)
                return false;
            downloadManager.downloadTrack(tracks[0].trackId);
            downloadDock.expanded = true;
            return true;
        }
        if (action === "collection-menu")
            return libraryPage.openCollectionActions();
        if (action === "collections")
            return libraryPage.openCollections();
        if (action === "mobile-queue") {
            listeningPanel.mobileQueue = true;
            return listeningPanel.mobileQueue;
        }
        return false;
    }
    // Открывает диалог создания нового плейлиста и сразу ставит фокус в поле имени.
    function createPlaylist() {
        playlistName.text = "";
        playlistDialog.open();
        playlistName.forceActiveFocus();
    }
    // Горячая клавиша пробела — воспроизведение/пауза (не в полях ввода).
    Shortcut {
        sequence: "Space"
        enabled: !root.typing
        onActivated: player.toggle()
    }
    Shortcut {
        sequence: "Ctrl+Right"
        onActivated: player.next()
    }
    Shortcut {
        sequence: "Ctrl+Left"
        onActivated: player.previous()
    }
    Shortcut {
        sequence: "Ctrl+O"
        onActivated: addFiles.open()
    }
    Shortcut {
        sequence: "Ctrl+F"
        onActivated: {
            root.listening = false;
            root.currentPage = 4;
            searchPage.focusSearch();
        }
    }
    Shortcut {
        sequence: "Escape"
        enabled: root.listening
        onActivated: root.listening = false
    }
    Shortcut {
        sequence: "Ctrl+L"
        onActivated: root.listening = !root.listening
    }
    Shortcut {
        sequence: "Ctrl+N"
        onActivated: root.createPlaylist()
    }
    // Компактный режим на узких экранах (например, мобильных).
    readonly property bool compact: width < 820
    // Отступы безопасной зоны (вырезы/системные панели).
    readonly property real safeTop: safeArea.SafeArea.margins.top
    readonly property real safeBottom: safeArea.SafeArea.margins.bottom
    readonly property real safeLeft: safeArea.SafeArea.margins.left
    readonly property real safeRight: safeArea.SafeArea.margins.right

    Theme {
        id: theme
    }
    Item {
        id: safeArea
        anchors.fill: parent
    }

    // Синхронизируем системную палитру с темой, чтобы стандартные контролы совпадали.
    palette.window: theme.background
    palette.windowText: theme.text
    palette.base: theme.surface
    palette.text: theme.text
    palette.button: theme.surfaceHigh
    palette.buttonText: theme.text
    palette.highlight: theme.accent
    palette.highlightedText: theme.textStrong
    palette.placeholderText: theme.subtext

    // На Android «назад» сначала возвращает на главную страницу вместо закрытия.
    onClosing: close => {
        if (Qt.platform.os === "android" && currentPage !== 0) {
            close.accepted = false;
            currentPage = 0;
        }
    }

    ColumnLayout {
        id: mainLayout
        anchors.fill: parent
        anchors.leftMargin: safeLeft
        anchors.rightMargin: safeRight
        spacing: 0

        RowLayout {
            clip: true
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: safeTop
            spacing: 0

            Components.NavigationPane {
                visible: !compact
                // Ширина навигации зависит от компактного режима.
                Layout.preferredWidth: settingsStore.compactNavigation ? 76 : 232
                collapsed: settingsStore.compactNavigation
                radius: 20
                clip: true
                Layout.leftMargin: 12
                Layout.topMargin: 12
                Layout.bottomMargin: 12
                Layout.rightMargin: 8
                Layout.fillHeight: true
                currentPage: root.currentPage
                onPageSelected: page => {
                    root.listening = false;
                    root.currentPage = page;
                }
                onCreatePlaylistRequested: root.createPlaylist()
            }

            StackLayout {
                visible: !root.listening
                Layout.fillWidth: true
                Layout.fillHeight: true
                // Порядок страниц соответствует индексам currentPage.
                currentIndex: root.currentPage

                Pages.HomePage {
                    id: homePage
                    compact: root.compact
                    onImportRequested: root.currentPage = 2
                    onLibraryRequested: root.currentPage = 1
                    onPlaylistsRequested: {
                        root.currentPage = 1;
                        libraryPage.openCollections();
                    }
                    onFilesRequested: addFiles.open()
                    onSearchRequested: {
                        root.currentPage = 4;
                        searchPage.focusSearch();
                    }
                    onDiscoveryRequested: {
                        root.currentPage = 4;
                        musicSearch.discover();
                    }
                }
                Pages.LibraryPage {
                    id: libraryPage
                    onCreatePlaylistRequested: root.createPlaylist()
                    compact: root.compact
                    onImportUrlRequested: root.currentPage = 2
                }
                Pages.ImportPage {
                    compact: root.compact
                }
                Pages.SettingsPage {
                    id: settingsPage
                    compact: root.compact
                }
                Pages.SearchPage {
                    id: searchPage
                    compact: root.compact
                    onLibraryRequested: {
                        libraryModel.playlistFilter = 0;
                        libraryModel.searchText = "";
                        root.currentPage = 1;
                    }
                }
            }

            Components.NowPlayingPanel {
                // Боковая панель очереди видна только на широких экранах и не в режиме прослушивания.
                visible: settingsStore.sideQueueVisible && !root.listening && !compact && root.width >= 1380 && root.currentPage <= 1
                Layout.preferredWidth: 280
                Layout.fillHeight: true
                Layout.margins: 16
                Layout.leftMargin: 16
            }
        }

        Components.PlayerBar {
            id: playbackBar
            Layout.fillWidth: true
            compact: root.compact
            onExpandRequested: root.listening = !root.listening
            onQueueRequested: {
                root.listening = true;
                listeningPanel.mobileQueue = true;
            }
            onEqualizerRequested: equalizerDialog.open()
        }

        Rectangle {
            // Нижняя навигация, показываемая только в компактном (мобильном) режиме.
            Layout.fillWidth: true
            Layout.preferredHeight: compact ? 64 + safeBottom : 0
            visible: compact
            color: theme.sidebar
            border.color: theme.line
            border.width: 1
            RowLayout {
                anchors.fill: parent
                anchors.bottomMargin: safeBottom
                spacing: 0
                Repeater {
                    model: [
                        {
                            label: "Главная",
                            icon: "music",
                            page: 0
                        },
                        {
                            label: "Поиск",
                            icon: "search",
                            page: 4
                        },
                        {
                            label: "Музыка",
                            icon: "library",
                            page: 1
                        },
                        {
                            label: "Импорт",
                            icon: "download",
                            page: 2
                        },
                        {
                            label: "Опции",
                            icon: "settings",
                            page: 3
                        }
                    ]
                    delegate: Button {
                        required property int index
                        required property var modelData
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        topPadding: 9
                        bottomPadding: 8
                        Accessible.name: modelData.label
                        background: Item {
                            Rectangle {
                                width: 54
                                height: 29
                                radius: 14
                                anchors.horizontalCenter: parent.horizontalCenter
                                y: 5
                                color: root.currentPage === modelData.page ? theme.selected : "transparent"
                            }
                        }
                        contentItem: Column {
                            spacing: 5
                            Common.AuroraIcon {
                                anchors.horizontalCenter: parent.horizontalCenter
                                name: modelData.icon
                                color: root.currentPage === modelData.page ? theme.accent : theme.subtext
                                size: 20
                            }
                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: modelData.label
                                color: root.currentPage === modelData.page ? theme.text : theme.subtext
                                font.pixelSize: 10
                            }
                        }
                        onClicked: {
                            root.listening = false;
                            root.currentPage = modelData.page;
                        }
                    }
                }
            }
        }
    }
    Components.ListeningView {
        id: listeningPanel
        anchors.fill: parent
        // Отступ снизу под панель воспроизведения (плюс мобильная навигация).
        anchors.bottomMargin: playbackBar.height + (root.compact ? 64 + safeBottom : 0)
        visible: root.listening
        compact: root.compact
        onCloseRequested: root.listening = false
    }
    Components.DownloadDock {
        id: downloadDock
        z: 20
        anchors.right: parent.right
        anchors.rightMargin: root.safeRight + 16
        anchors.bottom: parent.bottom
        // Отступ снизу учитывает панель воспроизведения и мобильную навигацию.
        anchors.bottomMargin: playbackBar.height + (root.compact ? 64 + root.safeBottom : 0) + 12
        // Ширина панели загрузок больше в развёрнутом виде.
        width: Math.min(root.width - root.safeLeft - root.safeRight - 32, expanded ? 720 : 600)
        height: implicitHeight
        compact: root.compact
    }
    // Перетаскивание аудиофайлов в окно запускает их импорт.
    DropArea {
        anchors.fill: parent
        onDropped: drop => {
            if (drop.hasUrls) {
                importManager.importFiles(drop.urls);
                root.currentPage = 1;
                drop.acceptProposedAction();
            }
        }
        Rectangle {
            anchors.fill: parent
            visible: parent.containsDrag
            color: "#dd14151c"
            border.color: theme.accent
            border.width: 2
            Text {
                anchors.centerIn: parent
                text: "Отпусти аудиофайлы здесь"
                color: theme.text
                font.pixelSize: 24
            }
        }
    }
    FileDialog {
        id: addFiles
        title: "Добавить музыку"
        fileMode: FileDialog.OpenFiles
        nameFilters: ["Аудио (*.mp3 *.flac *.wav *.ogg *.opus *.m4a *.aac *.wma *.aiff)"]
        onAccepted: {
            importManager.importFiles(selectedFiles);
            root.currentPage = 1;
        }
    }
    Common.AuroraDialog {
        id: playlistDialog
        uiTheme: theme
        parent: Overlay.overlay
        title: "Новый плейлист"
        standardButtons: Dialog.Ok | Dialog.Cancel
        Common.AuroraField {
            id: playlistName
            uiTheme: theme
            width: parent.width
            placeholderText: "Например, ночные прогулки"
            onAccepted: if (text.trim().length)
                playlistDialog.accept()
        }
        onAccepted: {
            if (playlistName.text.trim().length) {
                const id = libraryModel.createPlaylist(playlistName.text);
                if (id > 0) {
                    libraryModel.playlistFilter = id;
                    root.currentPage = 1;
                }
            }
        }
    }
    Common.AuroraToast {
        id: toast
        uiTheme: theme
        // Поднимаем тост над панелью воспроизведения и (при открытых) загрузках.
        anchors.bottomMargin: playbackBar.height + (downloadDock.visible ? downloadDock.height + 24 : 16) + (root.compact ? 64 + safeBottom : 0)
        duration: 5000
    }
    Components.SourceDialog {
        parent: Overlay.overlay
    }
    Components.EqualizerDialog {
        id: equalizerDialog
        parent: Overlay.overlay
    }
    Components.TrackEditor {
        parent: Overlay.overlay
    }
    Components.AlbumEditor {
        parent: Overlay.overlay
    }
    // Показываем тост при возникновении ошибки плеера.
    Connections {
        target: player
        function onLastErrorChanged() {
            if (player.lastError)
                toast.show(player.lastError);
        }
    }
    // Ошибки библиотеки тоже выводятся тостом.
    Connections {
        target: libraryModel
        function onLastErrorChanged() {
            if (libraryModel.lastError)
                toast.show(libraryModel.lastError);
        }
    }
    Connections {
        target: importManager
        function onLastErrorChanged() {
            if (importManager.lastError)
                toast.show(importManager.lastError);
        }
        // После импорта обновляем хранилище и сообщаем результат.
        function onImportCompleted(added, skipped) {
            storageManager.refresh();
            toast.show("Добавлено: " + added + " · пропущено: " + skipped);
        }
    }
    Connections {
        target: libraryExchange
        function onChanged() {
            if (libraryExchange.busy) {
                root.lastExchangeNotice = "";
                return;
            }
            // Показываем статус обмена только если он новый и мы не на странице импорта.
            if (libraryExchange.status && libraryExchange.status !== root.lastExchangeNotice) {
                root.lastExchangeNotice = libraryExchange.status;
                if (root.currentPage !== 2)
                    toast.show(libraryExchange.status);
            }
        }
        function onCompleted(id) {
            storageManager.refresh();
            libraryModel.playlistFilter = id;
            if (root.currentPage !== 2)
                root.currentPage = 1;
        }
    }

    // Нативный браузер авторизации активируется по запросу.
    Loader {
        active: webLogin.browserVisible && webLogin.nativeProfile !== null
        sourceComponent: Component {
            Web.NativeBrowser {}
        }
    }
}
