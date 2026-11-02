import QtQuick
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
    required property string format
    required property int bitrate
    required property var duration
    required property int rating
    signal playRequested(int row)
    signal rateRequested(int id, int value)
    signal removeRequested(int id)

    Theme { id: theme }
    width: ListView.view ? ListView.view.width : 600
    height: 72
    color: player.currentIndex === index ? theme.selected : hoverArea.containsMouse ? theme.hover : "transparent"
    radius: theme.radiusSmall

    function timeText(ms) {
        const total = Math.max(0, Math.floor(ms / 1000))
        return Math.floor(total / 60) + ":" + String(total % 60).padStart(2, "0")
    }

    MouseArea {
        id: hoverArea
        anchors.fill: parent
        hoverEnabled: true
        onDoubleClicked: root.playRequested(root.index)
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 8
        spacing: 12

        Common.AuroraButton {
            uiTheme: theme
            iconOnly: true
            iconName: player.currentIndex === root.index && player.playing ? "pause" : "play"
            implicitWidth: 40
            implicitHeight: 40
            onClicked: root.playRequested(root.index)
        }
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            Text {
                Layout.fillWidth: true
                text: root.title
                color: theme.text
                font.pixelSize: theme.bodySize
                font.weight: Font.Medium
                elide: Text.ElideRight
            }
            Text {
                Layout.fillWidth: true
                text: root.artist + (root.album.length ? "  ·  " + root.album : "")
                color: theme.subtext
                font.pixelSize: theme.captionSize
                elide: Text.ElideRight
            }
        }
        Text {
            Layout.preferredWidth: 92
            visible: root.width > 720
            text: root.source
            color: theme.subtext
            font.pixelSize: theme.captionSize
            elide: Text.ElideRight
        }
        Text {
            Layout.preferredWidth: 82
            visible: root.width > 820
            text: root.format + (root.bitrate ? " · " + root.bitrate + "k" : "")
            color: theme.subtext
            font.pixelSize: theme.captionSize
        }
        Text {
            Layout.preferredWidth: 42
            text: root.timeText(root.duration)
            color: theme.subtext
            font.pixelSize: theme.captionSize
        }
        Row {
            Layout.preferredWidth: 90
            visible: root.width > 560
            Repeater {
                model: 5
                delegate: Item {
                    width: 18
                    height: 36
                    Text {
                        anchors.centerIn: parent
                        text: index < root.rating ? "★" : "☆"
                        color: index < root.rating ? theme.accent2 : theme.subtext
                        font.pixelSize: 15
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: root.rateRequested(root.trackId, index + 1)
                    }
                }
            }
        }
        Common.AuroraButton {
            uiTheme: theme
            iconOnly: true
            iconName: "trash"
            implicitWidth: 36
            implicitHeight: 36
            onClicked: root.removeRequested(root.trackId)
        }
    }
}
