import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import ".."
import "../common" as Common

Rectangle {
    id: root

    required property var mix
    property color tint: "#6d5b9c"
    readonly property bool pending: (mix.tracks || []).some((track) => {
        const key = track.key || track.url || "";
        return key.length > 0 && (homeFeed.resolvingUrl === key || (player.streamKey === key && player.streamLoading));
    })

    signal playRequested()
    signal opened()

    width: 236
    height: 334
    radius: 18
    color: hover.containsMouse ? theme.hover : theme.surface
    border.color: theme.line
    ToolTip.visible: hover.containsMouse
    ToolTip.delay: 700
    ToolTip.text: root.mix.title + "\n" + (root.mix.description || root.mix.subtitle)

    Theme {
        id: theme
    }

    MouseArea {
        id: hover

        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.opened()
    }

    Rectangle {
        x: 12
        y: 12
        width: parent.width - 24
        height: 154
        radius: 12
        color: root.tint

        Grid {
            anchors.centerIn: parent
            columns: 2
            spacing: 4
            rotation: -9

            Repeater {
                model: 4

                AlbumArt {
                    required property int index
                    readonly property var art: root.mix.covers[index % Math.max(1, root.mix.covers.length)] || ({
                    })

                    width: 68
                    height: 68
                    cornerRadius: 8
                    path: art.path || ""
                    seed: art.seed || root.mix.title
                }

            }

        }

        Rectangle {
            anchors.fill: parent
            radius: parent.radius

            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: "#00000000"
                }

                GradientStop {
                    position: 1
                    color: "#b0000000"
                }

            }

        }

        Text {
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.margins: 12
            text: root.mix.count + " треков"
            color: "white"
            font.pixelSize: 11
            font.weight: Font.Medium
        }

        Common.AuroraButton {
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 10
            uiTheme: theme
            primary: true
            iconOnly: true
            iconName: root.pending ? "" : "play"
            text: root.pending ? "Подключаем аудио…" : "Слушать подборку"
            enabled: !root.pending
            implicitWidth: 38
            implicitHeight: 38
            onClicked: root.playRequested()

            Common.ProgressRing {
                anchors.centerIn: parent
                width: 22
                height: 22
                visible: root.pending
                indeterminate: true
                showLabel: false
                strokeWidth: 2
                color: theme.textStrong
            }

        }

    }

    Column {
        x: 14
        y: 179
        width: parent.width - 28
        spacing: 6

        Text {
            width: parent.width
            text: root.mix.title
            height: 40
            color: theme.text
            font.pixelSize: 15
            font.weight: Font.DemiBold
            elide: Text.ElideRight
            wrapMode: Text.Wrap
            maximumLineCount: 2
        }

        Text {
            width: parent.width
            text: root.mix.playlistName ? "Плейлист: " + root.mix.playlistName : "Микс Aurora · твоя библиотека"
            color: theme.subtext
            font.pixelSize: 11
            elide: Text.ElideRight
        }

        Flow {
            width: parent.width
            spacing: 5

            Repeater {
                model: root.mix.platforms || []

                Rectangle {
                    required property var modelData

                    height: 21
                    width: badge.implicitWidth + 12
                    radius: 7
                    color: theme.selected

                    Text {
                        id: badge

                        anchors.centerIn: parent
                        text: modelData.name + " · " + modelData.count
                        color: theme.accent
                        font.pixelSize: 9
                    }

                }

            }

        }

        Common.AuroraButton {
            uiTheme: theme
            implicitHeight: 30
            implicitWidth: parent.width
            iconName: "play"
            text: root.pending ? "Подключаем…" : root.mix.onlineCount > 0 ? "Слушать подборку" : "Слушать с устройства"
            enabled: !root.pending && (root.mix.onlineCount > 0 || root.mix.playableIds.length > 0)
            onClicked: root.playRequested()
        }

    }

}
