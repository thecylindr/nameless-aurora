// Диалог поиска аудио: показывает результаты по источникам, подбор и загрузку по ссылке.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../common" as Common

Dialog {
    id: root
    Theme {
        id: theme
    }
    property bool showLink: false
    property bool bandcampLink: false
    readonly property bool narrow: width < 560
    modal: true
    focus: true
    padding: narrow ? 16 : 24
    width: Math.min(860, Math.max(0, parent.width - 24))
    height: Math.min(800, Math.max(0, parent.height - 24))
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    onClosed: {
        searchDelay.stop();
        audioSources.close();
    }
    // Открывает диалог для конкретного трека.
    function showTrack(id) {
        audioSources.openTrack(id);
    }
    // Форматирует длительность в «м:сс», пустое значение показывается прочерком.
    function time(ms) {
        return ms > 0 ? Math.floor(ms / 60000) + ":" + String(Math.floor(ms / 1000) % 60).padStart(2, "0") : "—";
    }
    background: Rectangle {
        radius: 24
        color: theme.surface
        border.color: theme.line
    }
    Overlay.modal: Rectangle {
        color: "#ba06080f"
    }
    Connections {
        target: audioSources
        function onOpened() {
            query.text = (audioSources.target.artist || "") + " " + (audioSources.target.title || "");
            link.text = "";
            root.showLink = false;
            root.bandcampLink = false;
            root.open();
        }
        function onQueued() {
            root.close();
        }
    }
    header: Item {
        width: root.width
        implicitHeight: root.narrow ? 140 : 142
        Common.AuroraButton {
            x: root.width - width - root.padding
            y: root.padding
            uiTheme: theme
            iconName: "close"
            iconOnly: true
            text: "Закрыть"
            onClicked: root.close()
        }
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: root.padding
            spacing: 16
            RowLayout {
                Layout.fillWidth: true
                Layout.rightMargin: 56
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 5
                    Text {
                        text: "ДОПОЛНИ СВОЮ КОЛЛЕКЦИЮ"
                        color: theme.accent
                        font.pixelSize: 9
                        font.letterSpacing: 1.5
                    }
                    Text {
                        text: "Найти аудио"
                        color: theme.text
                        font.pixelSize: 25
                        font.weight: Font.DemiBold
                    }
                }
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: 12
                AlbumArt {
                    Layout.preferredWidth: 48
                    Layout.preferredHeight: 48
                    path: audioSources.target.coverPath || ""
                    seed: audioSources.target.title || "Aurora"
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    Text {
                        Layout.fillWidth: true
                        text: audioSources.target.title || ""
                        color: theme.text
                        font.pixelSize: 16
                        font.weight: Font.Medium
                        elide: Text.ElideRight
                    }
                    Text {
                        Layout.fillWidth: true
                        text: (audioSources.target.artist || "") + "  ·  " + root.time(audioSources.target.durationMs || 0)
                        color: theme.subtext
                        font.pixelSize: 12
                        elide: Text.ElideRight
                    }
                }
            }
        }
    }
    contentItem: ColumnLayout {
        spacing: 12
        RowLayout {
            Layout.fillWidth: true
            Common.AuroraField {
                id: query
                uiTheme: theme
                Layout.fillWidth: true
                iconName: "search"
                placeholderText: "Исполнитель и название"
                onTextEdited: searchDelay.restart()
                onAccepted: {
                    searchDelay.stop();
                    audioSources.search(audioSources.source || "Все включённые", text);
                }
            }
            Common.AuroraButton {
                uiTheme: theme
                iconName: audioSources.busy ? "close" : "search"
                iconOnly: true
                text: audioSources.busy ? "Остановить поиск" : "Найти"
                onClicked: {
                    searchDelay.stop();
                    audioSources.busy ? audioSources.cancel() : audioSources.search(audioSources.source || "Все включённые", query.text);
                }
            }
        }
        Flow {
            Layout.fillWidth: true
            spacing: 7
            visible: !root.narrow
            Repeater {
                model: ["Все включённые"].concat(audioSources.sources)
                delegate: Common.AuroraButton {
                    required property string modelData
                    uiTheme: theme
                    text: modelData
                    primary: audioSources.source === modelData
                    implicitHeight: 36
                    onClicked: {
                        searchDelay.stop();
                        audioSources.search(modelData, query.text);
                    }
                }
            }
            Common.AuroraButton {
                uiTheme: theme
                text: "Bandcamp · ссылка"
                implicitHeight: 36
                onClicked: {
                    root.bandcampLink = true;
                    root.showLink = true;
                    link.forceActiveFocus();
                }
            }
        }
        Common.AuroraCombo {
            uiTheme: theme
            visible: root.narrow
            Layout.fillWidth: true
            model: ["Все включённые"].concat(audioSources.sources)
            currentIndex: Math.max(0, audioSources.sources.indexOf(audioSources.source))
            onActivated: audioSources.search(currentText, query.text)
        }
        RowLayout {
            Layout.fillWidth: true
            Text {
                Layout.fillWidth: true
                text: downloadDependencies.busy ? downloadDependencies.statusText : audioSources.status
                color: theme.subtext
                font.pixelSize: 11
                wrapMode: Text.WordWrap
                maximumLineCount: root.narrow ? 3 : 2
                elide: Text.ElideRight
            }
            Text {
                visible: !root.narrow && audioSources.results.length > 0
                text: audioSources.results.length + " вариантов"
                color: theme.textMuted
                font.pixelSize: 11
            }
        }
        Common.AuroraProgress {
            uiTheme: theme
            Layout.fillWidth: true
            visible: audioSources.busy || downloadDependencies.busy
            indeterminate: true
        }
        ListView {
            id: results
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 8
            model: audioSources.results
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: Common.AuroraScrollBar {
                uiTheme: theme
            }
            delegate: Rectangle {
                required property var modelData
                required property int index
                width: results.width - 10
                height: root.narrow ? 98 : 88
                radius: 15
                color: rowHover.hovered ? theme.hover : theme.surfaceHigh
                border.color: index === 0 && modelData.score >= 90 ? Qt.rgba(theme.accent.r, theme.accent.g, theme.accent.b, .38) : "transparent"
                HoverHandler {
                    id: rowHover
                }
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12
                    Rectangle {
                        visible: !root.narrow
                        Layout.preferredWidth: 42
                        Layout.preferredHeight: 42
                        radius: 13
                        color: theme.selected
                        Common.AuroraIcon {
                            anchors.centerIn: parent
                            name: "music"
                            color: theme.accent
                            size: 20
                        }
                    }
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 6
                        Text {
                            Layout.fillWidth: true
                            text: modelData.title
                            color: theme.text
                            font.pixelSize: 13
                            font.weight: Font.Medium
                            maximumLineCount: 2
                            wrapMode: Text.WordWrap
                            elide: Text.ElideRight
                        }
                        Text {
                            Layout.fillWidth: true
                            text: modelData.source + " · " + (modelData.artist || "") + " · " + root.time(modelData.durationMs || 0)
                            color: theme.subtext
                            font.pixelSize: 11
                            elide: Text.ElideRight
                        }
                        Text {
                            Layout.fillWidth: true
                            // Оценка совпадения влияет и на подпись, и на цвет строки.
                            text: modelData.score >= 90 ? "Близкое совпадение" : modelData.score >= 65 ? "Проверь исполнителя и версию" : "Возможно, другая версия"
                            color: modelData.score >= 90 ? theme.good : theme.warn
                            font.pixelSize: 10
                            elide: Text.ElideRight
                        }
                    }
                    Common.AuroraButton {
                        uiTheme: theme
                        iconName: "download"
                        text: root.narrow ? "" : "Скачать"
                        iconOnly: root.narrow
                        primary: index === 0 && modelData.score >= 90
                        enabled: !audioSources.busy || audioSources.source === "Все включённые"
                        onClicked: audioSources.choose(index)
                    }
                }
            }
            Column {
                anchors.centerIn: parent
                width: Math.min(parent.width - 30, 380)
                spacing: 12
                visible: !audioSources.busy && audioSources.results.length === 0
                Text {
                    width: parent.width
                    text: "Одна песня — разные источники"
                    color: theme.text
                    font.pixelSize: 18
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                }
                Text {
                    width: parent.width
                    text: "Запусти автоподбор или выбери сервис сверху. Найденный файл дополнит этот трек в плейлистах."
                    color: theme.subtext
                    font.pixelSize: 13
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    lineHeight: 1.4
                }
            }
        }
    }
    // Поиск с задержкой 500 мс, чтобы не дёргать запрос на каждый введённый символ.
    Timer {
        id: searchDelay
        interval: 500
        onTriggered: if (root.visible && query.text.trim().length > 1)
            audioSources.search(audioSources.source || "Все включённые", query.text)
    }
    footer: Item {
        implicitHeight: root.showLink ? 178 : 130
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: root.padding
            spacing: 10
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 1
                color: theme.line
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                Common.AuroraButton {
                    uiTheme: theme
                    Layout.fillWidth: true
                    Layout.minimumWidth: 0
                    primary: true
                    iconName: "search"
                    text: "Подобрать и скачать"
                    onClicked: {
                        const id = audioSources.target.trackId;
                        root.close();
                        audioSources.recoverTrack(id);
                    }
                }
                Common.AuroraButton {
                    uiTheme: theme
                    iconName: "link"
                    iconOnly: true
                    text: "По своей ссылке"
                    onClicked: root.showLink = !root.showLink
                }
                Common.AuroraButton {
                    uiTheme: theme
                    iconName: "refresh"
                    iconOnly: true
                    text: "Повторить исходный источник"
                    enabled: !audioSources.busy
                    onClicked: {
                        downloadManager.downloadTrack(audioSources.target.trackId);
                        root.close();
                    }
                }
            }
            RowLayout {
                visible: root.showLink
                Layout.fillWidth: true
                Common.AuroraField {
                    id: link
                    uiTheme: theme
                    Layout.fillWidth: true
                    placeholderText: root.bandcampLink ? "https://artist.bandcamp.com/track/…" : "Ссылка на запись или MP3"
                    onAccepted: audioSources.downloadLink(text)
                }
                Common.AuroraButton {
                    uiTheme: theme
                    iconName: "download"
                    iconOnly: true
                    text: "Скачать по ссылке"
                    enabled: !audioSources.busy && link.text.trim().length > 0
                    onClicked: audioSources.downloadLink(link.text)
                }
            }
            Text {
                Layout.fillWidth: true
                text: "Автоподбор сверяет название, версию и длительность. Плейлисты, обложка и оценки сохранятся."
                color: theme.textMuted
                font.pixelSize: 10
                maximumLineCount: 2
                wrapMode: Text.WordWrap
                elide: Text.ElideRight
            }
        }
    }
}
