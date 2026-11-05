// Строка трека в списке: обложка, метаданные, статус загрузки и меню действий.
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import "../common" as Common

Rectangle {
    id: root
    required property int index
    required property int trackId
    required property string title
    required property string artist
    required property string album
    required property string source
    required property string sourceUrl
    required property string coverPath
    required property string format
    required property int bitrate
    required property var duration
    required property int rating
    required property bool available
    // Скрываем элементы строки, которые обрезаны краем списка и могли бы просвечивать в плеер ниже (включая Shape-иконки на программном рендере).
    readonly property bool controlsInView: !ListView.view || (y + height / 2 - 18 >= ListView.view.contentY && y + height / 2 + 18 <= ListView.view.contentY + ListView.view.height)
    // Текущее состояние загрузки этого трека из DownloadManager.
    property var transfer: downloadManager.trackState(trackId)
    Connections {
        target: downloadManager
        function onSummaryChanged() {
            root.transfer = downloadManager.trackState(root.trackId);
        }
    }
    // Открывает меню действий и возвращает результат, чтобы страница могла подтвердить вызов.
    function openActions() {
        menu.open();
        return menu.visible;
    }
    Theme {
        id: theme
    }
    width: ListView.view ? ListView.view.width : 600
    height: root.width > 650 ? 76 : 82
    radius: 10
    color: player.currentTrackId === trackId ? theme.selected : hover.containsMouse ? theme.hover : "transparent"
    MouseArea {
        id: hover
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onDoubleClicked: root.available ? player.playTrackId(root.trackId) : audioSources.openTrack(root.trackId)
        onClicked: mouse => {
            if (mouse.button === Qt.RightButton)
                menu.popup();
        }
    }
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 6
        spacing: 12
        Item {
            Layout.preferredWidth: 44
            Layout.preferredHeight: 44
            AlbumArt {
                anchors.fill: parent
                path: root.coverPath
                seed: root.title
                cornerRadius: 8
            }
            Rectangle {
                anchors.fill: parent
                radius: 8
                color: "#88000000"
                visible: hover.containsMouse || player.currentTrackId === root.trackId
            }
            Common.AuroraIcon {
                anchors.centerIn: parent
                name: player.currentTrackId === root.trackId && player.playing ? "pause" : "play"
                size: 18
                color: "white"
                visible: hover.containsMouse || player.currentTrackId === root.trackId
            }
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: root.available ? player.playTrackId(root.trackId) : audioSources.openTrack(root.trackId)
            }
        }
        ColumnLayout {
            Layout.preferredWidth: root.width > 900 ? Math.min(560, root.width * .46) : -1
            Layout.maximumWidth: root.width > 850 ? Math.min(680, root.width * .46) : Infinity
            Layout.fillWidth: true
            spacing: 5
            Text {
                Layout.fillWidth: true
                text: root.title
                color: player.currentTrackId === root.trackId ? theme.accent : theme.text
                font.pixelSize: 14
                font.weight: Font.Medium
                elide: Text.ElideRight
            }
            Text {
                Layout.fillWidth: true
                text: root.artist + (root.width < 850 && root.album.length ? " · " + root.album : "")
                color: theme.subtext
                font.pixelSize: 12
                elide: Text.ElideRight
            }
        }
        ColumnLayout {
            Layout.preferredWidth: root.width > 1250 ? 240 : 170
            Layout.minimumWidth: Layout.preferredWidth
            Layout.fillWidth: true
            visible: root.width > 850
            spacing: 5
            Text {
                Layout.fillWidth: true
                text: root.album || "Без альбома"
                color: root.album.length ? theme.subtext : theme.textMuted
                font.pixelSize: 12
                elide: Text.ElideRight
            }
            Text {
                Layout.fillWidth: true
                text: root.source
                visible: root.source.length > 0
                color: theme.textMuted
                font.pixelSize: 10
                elide: Text.ElideRight
            }
        }
        Item {
            Layout.preferredWidth: 160
            Layout.minimumWidth: 160
            implicitHeight: 38
            visible: root.width > 540
            opacity: root.controlsInView ? 1 : 0
            enabled: root.controlsInView
            Row {
                anchors.verticalCenter: parent.verticalCenter
                spacing: 6
                visible: root.available
                Rectangle {
                    width: 6
                    height: 6
                    radius: 3
                    color: theme.good
                    anchors.verticalCenter: parent.verticalCenter
                }
                Text {
                    text: root.format + (root.bitrate ? " · " + root.bitrate + " kbps" : "")
                    color: theme.textMuted
                    font.pixelSize: 10
                }
            }
            Common.AuroraButton {
                uiTheme: theme
                anchors.verticalCenter: parent.verticalCenter
                text: ["queued", "resolving", "downloading", "importing"].indexOf(root.transfer.status) >= 0 ? "Загружается" : root.transfer.status === "failed" ? "Выбрать аудио" : "Докачать"
                iconName: "download"
                implicitHeight: 30
                visible: !root.available
                enabled: ["queued", "resolving", "downloading", "importing"].indexOf(root.transfer.status) < 0
                onClicked: audioSources.openTrack(root.trackId)
            }
        }
        Common.AuroraButton {
            uiTheme: theme
            iconName: "download"
            iconOnly: true
            text: "Найти аудио"
            visible: !root.available && root.width <= 540
            opacity: root.controlsInView ? 1 : 0
            enabled: root.controlsInView
            implicitWidth: 30
            implicitHeight: 30
            onClicked: audioSources.openTrack(root.trackId)
        }
        Common.AuroraButton {
            uiTheme: theme
            iconOnly: true
            iconName: "heart"
            text: root.rating >= 4 ? "Убрать из любимых" : "Мне нравится"
            implicitWidth: 32
            implicitHeight: 32
            primary: root.rating >= 4
            visible: root.width > 380
            opacity: root.controlsInView ? 1 : 0
            enabled: root.controlsInView
            onClicked: libraryModel.setRating(root.trackId, root.rating >= 4 ? 0 : 5)
        }
        Text {
            Layout.preferredWidth: 36
            text: root.duration > 0 ? Math.floor(root.duration / 60000) + ":" + String(Math.floor(root.duration / 1000) % 60).padStart(2, "0") : "—"
            color: theme.subtext
            font.pixelSize: 11
            horizontalAlignment: Text.AlignRight
        }
        Common.AuroraButton {
            uiTheme: theme
            iconOnly: true
            iconName: "more"
            text: "Действия с треком"
            implicitWidth: 32
            implicitHeight: 32
            opacity: root.controlsInView ? 1 : 0
            enabled: root.controlsInView
            onClicked: menu.popup()
        }
    }
    Common.AuroraMenu {
        id: menu
        uiTheme: theme
        objectName: "trackActions"
        Common.AuroraMenuItem {
            uiTheme: theme
            text: root.rating >= 4 ? "Убрать из любимых" : "Мне нравится"
            onTriggered: libraryModel.setRating(root.trackId, root.rating >= 4 ? 0 : 5)
        }
        Common.AuroraMenuItem {
            uiTheme: theme
            text: "Добавить в очередь"
            enabled: root.available
            onTriggered: player.enqueue(root.trackId)
        }
        Common.AuroraMenu {
            id: playlistsMenu
            uiTheme: theme
            title: "Добавить в плейлист"
            enabled: libraryModel.playlists.length > 0
            Instantiator {
                model: libraryModel.playlists
                delegate: Common.AuroraMenuItem {
                    uiTheme: theme
                    required property var modelData
                    text: modelData.name
                    onTriggered: libraryModel.addToPlaylist(modelData.id, root.trackId)
                }
                onObjectAdded: (index, object) => playlistsMenu.insertItem(index, object)
                onObjectRemoved: (index, object) => playlistsMenu.removeItem(object)
            }
        }
        Common.AuroraMenu {
            id: ratingsMenu
            uiTheme: theme
            title: "Оценка · " + root.rating + "/5"
            Instantiator {
                model: 6
                delegate: Common.AuroraMenuItem {
                    uiTheme: theme
                    required property int index
                    text: index === 0 ? "Без оценки" : "★".repeat(index)
                    onTriggered: libraryModel.setRating(root.trackId, index)
                }
                onObjectAdded: (index, object) => ratingsMenu.insertItem(index, object)
                onObjectRemoved: (index, object) => ratingsMenu.removeItem(object)
            }
        }
        MenuSeparator {}
        Common.AuroraMenuItem {
            uiTheme: theme
            text: "Редактировать трек…"
            onTriggered: libraryModel.requestEdit(root.trackId)
        }
        Common.AuroraMenuItem {
            uiTheme: theme
            text: root.album.length ? "Редактировать альбом…" : "Собрать альбом…"
            onTriggered: libraryModel.requestAlbumEdit(root.album)
        }
        Common.AuroraMenuItem {
            uiTheme: theme
            text: "Привязать аудиофайл…"
            onTriggered: attachDialog.open()
        }
        Common.AuroraMenuItem {
            uiTheme: theme
            text: "Открыть в сервисе"
            enabled: /^https?:\/\//.test(root.sourceUrl)
            onTriggered: Qt.openUrlExternally(root.sourceUrl)
        }
        Common.AuroraMenuItem {
            uiTheme: theme
            text: "Докачать / выбрать источник…"
            enabled: !root.available
            onTriggered: audioSources.openTrack(root.trackId)
        }
        MenuSeparator {}
        Common.AuroraMenuItem {
            uiTheme: theme
            text: "Убрать из этого плейлиста"
            enabled: libraryModel.playlistFilter > 0
            onTriggered: libraryModel.removeFromPlaylist(libraryModel.playlistFilter, root.trackId)
        }
        Common.AuroraMenuItem {
            uiTheme: theme
            text: "Удалить из медиатеки"
            onTriggered: libraryModel.removeTrack(root.trackId)
        }
    }
    FileDialog {
        id: attachDialog
        title: "Аудиофайл для «" + root.title + "»"
        nameFilters: ["Аудио (*.mp3 *.flac *.wav *.ogg *.opus *.m4a *.aac *.wma *.aiff)"]
        onAccepted: importManager.attachFile(root.trackId, selectedFile)
    }
}
