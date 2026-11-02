import QtQuick
import QtQuick.Layouts
import "../common" as Common

Common.AuroraCard {
    id: root
    Theme { id: theme }
    uiTheme: theme
    padding: 24

    ColumnLayout {
        anchors.fill: parent
        spacing: 18
        Text { text: "СЕЙЧАС ИГРАЕТ"; color: theme.subtext; font.pixelSize: theme.captionSize }
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: Math.min(width, 250)
            radius: theme.radiusLarge
            gradient: Gradient {
                GradientStop { position: 0; color: player.hasTrack ? theme.accent2 : theme.surfaceHigh }
                GradientStop { position: 1; color: player.hasTrack ? theme.accent : theme.surface }
            }
            Image {
                anchors.fill: parent
                source: player.currentCoverPath ? encodeURI("file://" + player.currentCoverPath) : ""
                fillMode: Image.PreserveAspectCrop
                asynchronous: true
            }
            Common.AuroraIcon {
                anchors.centerIn: parent
                name: "music"
                color: theme.text
                size: 64
                visible: player.currentCoverPath.length === 0
            }
        }
        Text {
            Layout.fillWidth: true
            text: player.hasTrack ? player.currentTitle : "Ваша музыка"
            color: theme.text
            font.pixelSize: 22
            font.weight: Font.DemiBold
            wrapMode: Text.WordWrap
        }
        Text {
            Layout.fillWidth: true
            text: player.hasTrack ? player.currentArtist : "Локально и без лишних окон"
            color: theme.subtext
            font.pixelSize: theme.bodySize
            wrapMode: Text.WordWrap
        }
        Item { Layout.fillHeight: true }
        Text {
            Layout.fillWidth: true
            text: player.hasTrack ? player.currentFormat + " · локальный файл" : "Qt Multimedia"
            color: theme.subtext
            font.pixelSize: theme.captionSize
        }
    }
}
