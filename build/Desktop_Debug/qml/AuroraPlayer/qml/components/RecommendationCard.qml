import QtQuick
import QtQuick.Layouts
import "../common" as Common

Common.AuroraCard {
    id: root
    property var trackData
    signal clicked()
    Theme { id: theme }
    uiTheme: theme
    padding: 10
    implicitWidth: 176
    implicitHeight: 236

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.padding
        spacing: 9
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: width
            radius: theme.radiusSmall
            color: theme.surfaceHigh
            clip: true
            gradient: Gradient {
                GradientStop { position: 0; color: theme.accent2 }
                GradientStop { position: 1; color: theme.accent }
            }
            Image {
                id: coverImage
                anchors.fill: parent
                source: root.trackData.coverPath ? encodeURI("file://" + root.trackData.coverPath) : ""
                fillMode: Image.PreserveAspectCrop
                asynchronous: true
                visible: status === Image.Ready
            }
            Common.AuroraIcon {
                anchors.centerIn: parent
                name: "music"
                color: theme.text
                size: 40
                visible: !coverImage.visible
            }
            Common.AuroraButton {
                uiTheme: theme
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 8
                primary: true
                iconOnly: true
                iconName: "play"
                implicitWidth: 40
                implicitHeight: 40
                onClicked: root.clicked()
            }
            MouseArea {
                anchors.fill: parent
                onClicked: root.clicked()
            }
        }
        Text {
            Layout.fillWidth: true
            text: root.trackData.title
            color: theme.text
            font.pixelSize: theme.bodySize
            font.weight: Font.Medium
            elide: Text.ElideRight
        }
        Text {
            Layout.fillWidth: true
            text: root.trackData.artist
            color: theme.subtext
            font.pixelSize: theme.captionSize
            elide: Text.ElideRight
        }
    }
}
