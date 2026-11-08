import QtQuick
import QtQuick.Layouts
import ".."
import "../common" as Common

Rectangle {
    id: root

    required property string title
    property bool compact: false
    property bool connected: false
    property bool busy: false
    property string status: ""
    property var tracks: []

    signal connectRequested()
    signal refreshRequested()
    signal playRequested(int index)
    signal opened()

    implicitHeight: content.implicitHeight + 36
    radius: 18
    color: theme.surface
    border.color: theme.line

    Theme {
        id: theme
    }

    ColumnLayout {
        id: content

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 18
        spacing: 14

        RowLayout {
            Layout.fillWidth: true

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 5

                Text {
                    Layout.fillWidth: true
                    text: root.title
                    color: theme.text
                    font.pixelSize: root.compact ? 16 : 20
                    font.weight: Font.DemiBold
                    wrapMode: Text.WordWrap
                }

                Text {
                    Layout.fillWidth: true
                    text: root.status || (root.connected ? "Персональные рекомендации твоего аккаунта" : "Подключи аккаунт, чтобы открыть свою музыку")
                    color: theme.subtext
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                }

            }

            Common.AuroraButton {
                uiTheme: theme
                iconName: "play"
                iconOnly: true
                text: "Слушать рекомендации"
                visible: root.tracks.length > 0
                onClicked: root.playRequested(0)
            }

            Common.AuroraButton {
                uiTheme: theme
                iconName: root.connected ? "refresh" : "link"
                iconOnly: root.compact
                text: root.connected ? "Обновить" : "Подключить"
                enabled: !root.busy
                onClicked: root.connected ? root.refreshRequested() : root.connectRequested()
            }

        }

        Common.AuroraButton {
            uiTheme: theme
            iconName: "library"
            text: "Открыть плейлист · " + root.tracks.length + " треков"
            visible: root.tracks.length > 0
            onClicked: root.opened()
        }

        ListView {
            Layout.fillWidth: true
            implicitHeight: 230
            visible: root.tracks.length > 0
            orientation: ListView.Horizontal
            model: root.tracks
            spacing: 14
            clip: true
            boundsBehavior: Flickable.StopAtBounds

            delegate: RecommendationCard {
                required property var modelData
                required property int index

                trackData: modelData
                onClicked: root.playRequested(index)
            }

        }

    }

}
