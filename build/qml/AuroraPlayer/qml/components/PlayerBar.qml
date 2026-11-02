import QtQuick
import QtQuick.Layouts
import "../common" as Common

Rectangle {
    id: root
    property bool compact: false
    Theme { id: theme }
    color: theme.sidebar
    border.color: theme.line
    border.width: 1
    implicitHeight: compact ? 78 : 96

    function timeText(ms) {
        const total = Math.max(0, Math.floor(ms / 1000))
        return Math.floor(total / 60) + ":" + String(total % 60).padStart(2, "0")
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: compact ? 8 : 14
        spacing: compact ? 3 : 7

        RowLayout {
            Layout.fillWidth: true
            spacing: 12
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2
                Text {
                    Layout.fillWidth: true
                    text: player.hasTrack ? player.currentTitle : "Выберите музыку"
                    color: theme.text
                    font.pixelSize: theme.bodySize
                    font.weight: Font.Medium
                    elide: Text.ElideRight
                }
                Text {
                    Layout.fillWidth: true
                    text: player.hasTrack ? player.currentArtist : "Aurora Player"
                    color: theme.subtext
                    font.pixelSize: theme.captionSize
                    elide: Text.ElideRight
                    visible: !compact
                }
            }
            RowLayout {
                spacing: 4
                Common.AuroraButton {
                    uiTheme: theme
                    iconOnly: true
                    iconName: "previous"
                    implicitWidth: 40
                    onClicked: player.previous()
                }
                Common.AuroraButton {
                    uiTheme: theme
                    primary: true
                    iconOnly: true
                    iconName: player.playing ? "pause" : "play"
                    implicitWidth: compact ? 44 : 48
                    onClicked: player.toggle()
                }
                Common.AuroraButton {
                    uiTheme: theme
                    iconOnly: true
                    iconName: "next"
                    implicitWidth: 40
                    onClicked: player.next()
                }
            }
            RowLayout {
                Layout.preferredWidth: compact ? 0 : 150
                visible: !compact
                spacing: 8
                Common.AuroraIcon { name: "music"; color: theme.subtext; size: 17 }
                Common.AuroraSlider {
                    uiTheme: theme
                    Layout.fillWidth: true
                    from: 0
                    to: 1
                    value: player.volume
                    onMoved: player.volume = value
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            Text { text: root.timeText(player.position); color: theme.subtext; font.pixelSize: 9 }
            Common.AuroraSlider {
                uiTheme: theme
                Layout.fillWidth: true
                from: 0
                to: Math.max(1, player.duration)
                value: player.position
                onMoved: player.seek(value)
            }
            Text { text: root.timeText(player.duration); color: theme.subtext; font.pixelSize: 9 }
        }
    }
}
