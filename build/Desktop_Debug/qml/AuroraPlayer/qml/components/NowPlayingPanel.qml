// Панель «Сейчас играет» и очередь воспроизведения.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../common" as Common

Rectangle {
    id: root
    property bool queueOnly: false
    // В режиме «только очередь» текущий трек исключается из списка.
    readonly property var queueItems: queueOnly ? player.queue.filter(track => track.trackId !== player.currentTrackId) : player.queue
    Theme {
        id: theme
    }
    color: theme.surface
    radius: 22
    border.color: theme.line
    border.width: 1
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 18
        spacing: 18
        RowLayout {
            Layout.fillWidth: true
            visible: !root.queueOnly
            Text {
                text: "Сейчас играет"
                color: theme.text
                font.pixelSize: 14
                font.weight: Font.Medium
                Layout.fillWidth: true
            }
            Common.AuroraIcon {
                name: "music"
                color: theme.accent
                size: 18
            }
        }
        AlbumArt {
            visible: !root.queueOnly
            Layout.fillWidth: true
            Layout.preferredHeight: width
            path: player.currentCoverPath
            seed: player.hasTrack ? player.currentTitle : "Aurora"
            cornerRadius: 14
        }
        ColumnLayout {
            visible: !root.queueOnly
            Layout.fillWidth: true
            spacing: 7
            Text {
                Layout.fillWidth: true
                text: player.hasTrack ? player.currentTitle : "Всё начинается с трека"
                color: theme.text
                font.pixelSize: 20
                font.weight: Font.DemiBold
                wrapMode: Text.WordWrap
                maximumLineCount: 2
                elide: Text.ElideRight
            }
            Text {
                Layout.fillWidth: true
                text: player.hasTrack ? player.currentArtist : "Добавь свою музыку"
                color: theme.subtext
                font.pixelSize: 12
                elide: Text.ElideRight
            }
        }
        Rectangle {
            visible: !root.queueOnly
            Layout.fillWidth: true
            implicitHeight: 1
            color: theme.line
        }
        RowLayout {
            Layout.fillWidth: true
            Text {
                text: "Очередь"
                color: theme.text
                font.pixelSize: 14
                font.weight: Font.Medium
                Layout.fillWidth: true
            }
            Text {
                text: root.queueItems.length
                color: theme.textMuted
                font.pixelSize: 12
            }
        }
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: root.queueItems
            clip: true
            spacing: 2
            delegate: ItemDelegate {
                required property var modelData
                width: ListView.view.width
                height: 56
                background: Rectangle {
                    radius: 8
                    color: parent.hovered ? theme.hover : modelData.trackId === player.currentTrackId ? theme.selected : "transparent"
                }
                contentItem: RowLayout {
                    spacing: 10
                    AlbumArt {
                        Layout.preferredWidth: 34
                        Layout.preferredHeight: 34
                        path: modelData.coverPath
                        seed: modelData.title
                        cornerRadius: 6
                    }
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 5
                        Text {
                            Layout.fillWidth: true
                            text: modelData.title
                            color: modelData.trackId === player.currentTrackId ? theme.accent : theme.text
                            font.pixelSize: 12
                            elide: Text.ElideRight
                        }
                        Text {
                            Layout.fillWidth: true
                            text: modelData.artist
                            color: theme.subtext
                            font.pixelSize: 10
                            elide: Text.ElideRight
                        }
                    }
                    Common.AuroraButton {
                        uiTheme: theme
                        iconOnly: true
                        iconName: "close"
                        text: "Убрать из очереди"
                        implicitWidth: 24
                        implicitHeight: 24
                        visible: modelData.trackId !== player.currentTrackId
                        onClicked: player.removeFromQueue(modelData.trackId)
                    }
                }
                onClicked: player.playQueueTrack(modelData.trackId)
            }
            Text {
                anchors.fill: parent
                visible: root.queueItems.length === 0
                text: "Здесь будет следующий трек.\nДобавь музыку через меню ⋯"
                color: theme.textMuted
                font.pixelSize: 12
                wrapMode: Text.WordWrap
                lineHeight: 1.5
            }
        }
        Text {
            text: player.hasTrack ? player.currentFormat + (player.streaming ? "  /  ОНЛАЙН" : "  /  НА ЭТОМ УСТРОЙСТВЕ") : "СЛУШАЙ В СВОЁМ РИТМЕ"
            color: theme.textMuted
            font.pixelSize: 8
            font.letterSpacing: 1
        }
    }
}
