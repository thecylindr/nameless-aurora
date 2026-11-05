// Карточка рекомендации: обложка с кнопкой воспроизведения и подписи названия и исполнителя.
import QtQuick
import QtQuick.Layouts
import "../common" as Common

Item {
    id: root
    property var trackData
    signal clicked
    width: 172
    height: 230
    Theme {
        id: theme
    }
    ColumnLayout {
        anchors.fill: parent
        spacing: 8
        AlbumArt {
            Layout.fillWidth: true
            Layout.preferredHeight: 172
            path: root.trackData.coverPath || ""
            seed: root.trackData.album || root.trackData.title
            Common.AuroraButton {
                uiTheme: theme
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 10
                primary: true
                iconOnly: true
                iconName: "play"
                text: "Слушать"
                implicitWidth: 40
                implicitHeight: 40
                visible: hover.containsMouse || player.currentTrackId === root.trackData.trackId
                onClicked: root.clicked()
            }
            MouseArea {
                id: hover
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: root.clicked()
            }
        }
        Text {
            Layout.fillWidth: true
            text: root.trackData.title
            color: theme.text
            font.pixelSize: 14
            font.weight: Font.Medium
            elide: Text.ElideRight
        }
        Text {
            Layout.fillWidth: true
            text: root.trackData.artist
            color: theme.subtext
            font.pixelSize: 12
            elide: Text.ElideRight
        }
    }
}
