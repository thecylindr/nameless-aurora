// Экран прослушивания: крупная обложка, сфера-атмосфера и управление воспроизведением поверх фона.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../common" as Common

Item {
    id: root
    objectName: "listeningView"
    property bool compact: false
    property bool mobileQueue: false
    signal closeRequested
    Theme {
        id: theme
    }
    clip: true
    AmbientBackdrop {
        anchors.fill: parent
        intensity: settingsStore.atmosphereIntensity * .92
    }
    // Затемнение по краям, чтобы текст и обложка читались поверх фоновой атмосферы.
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop {
                position: 0
                color: Qt.rgba(theme.base.r, theme.base.g, theme.base.b, .36)
            }
            GradientStop {
                position: .65
                color: "transparent"
            }
            GradientStop {
                position: 1
                color: Qt.rgba(theme.base.r, theme.base.g, theme.base.b, .5)
            }
        }
    }
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.compact ? 20 : 36
        spacing: 12
        RowLayout {
            Layout.fillWidth: true
            Layout.minimumWidth: root.width - (root.compact ? 40 : 72)
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 7
                Text {
                    text: "A U R O R A"
                    color: theme.subtext
                    font.pixelSize: 9
                    font.letterSpacing: 2
                }
                Text {
                    text: player.playing ? "Сейчас играет" : "Твоя музыка"
                    color: theme.text
                    font.pixelSize: root.compact ? 20 : 25
                    font.weight: Font.Medium
                }
            }
            Item {
                Layout.preferredWidth: root.compact ? 96 : 48
                Layout.preferredHeight: 44
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.maximumWidth: 1200
            Layout.alignment: Qt.AlignHCenter
            spacing: 32
            visible: !root.compact || !root.mobileQueue
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 800
                spacing: 16
                Item {
                    id: stage
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 160
                    AtmosphereOrb {
                        id: atmosphere
                        width: Math.min(stage.width * 1.45, stage.height * 1.8, 1040)
                        height: width
                        anchors.centerIn: parent
                        opacity: .86
                        interactive: false
                    }
                    Rectangle {
                        width: art.width - 14
                        height: width
                        radius: 24
                        anchors.horizontalCenter: art.horizontalCenter
                        anchors.top: art.top
                        anchors.topMargin: 24
                        color: Qt.rgba(0, 0, 0, .25)
                    }
                    Rectangle {
                        width: art.width + 12
                        height: width
                        radius: 26
                        anchors.centerIn: art
                        color: Qt.rgba(player.atmosphereColor.r, player.atmosphereColor.g, player.atmosphereColor.b, .08)
                        border.color: Qt.rgba(player.atmosphereColor.r, player.atmosphereColor.g, player.atmosphereColor.b, .18)
                    }
                    AlbumArt {
                        id: art
                        width: Math.max(110, Math.min(stage.width * .68, stage.height * .76, root.compact ? 300 : 410))
                        height: width
                        anchors.centerIn: parent
                        cornerRadius: 20
                        path: player.currentCoverPath
                        seed: player.currentTitle || "Aurora"
                        // Едва заметное увеличение обложки в такт ударам.
                        scale: 1 + (atmosphere.reacting && settingsStore.beatEffects ? atmosphere.pulse * 0.006 : 0)
                    }
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.maximumWidth: 660
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 9
                    Text {
                        Layout.fillWidth: true
                        text: player.currentTitle || "Поймай свою волну"
                        color: theme.text
                        font.pixelSize: root.compact ? 24 : 34
                        font.weight: Font.DemiBold
                        horizontalAlignment: Text.AlignHCenter
                        maximumLineCount: 2
                        wrapMode: Text.WordWrap
                        elide: Text.ElideRight
                    }
                    Text {
                        Layout.fillWidth: true
                        text: player.currentArtist || "Выбери трек в коллекции"
                        color: theme.subtext
                        font.pixelSize: root.compact ? 13 : 16
                        horizontalAlignment: Text.AlignHCenter
                        elide: Text.ElideRight
                    }
                }
                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 10
                    Common.AuroraButton {
                        uiTheme: theme
                        iconOnly: true
                        iconName: "heart"
                        text: "Мне нравится"
                        primary: player.favorite
                        enabled: player.hasTrack
                        onClicked: player.toggleFavorite()
                    }
                    Common.AuroraButton {
                        uiTheme: theme
                        iconOnly: true
                        iconName: "shuffle"
                        text: "Перемешать"
                        primary: player.shuffle
                        onClicked: player.shuffle = !player.shuffle
                    }
                    Common.AuroraButton {
                        uiTheme: theme
                        iconOnly: true
                        iconName: "repeat"
                        text: "Повтор"
                        primary: player.repeatMode > 0
                        onClicked: player.repeatMode = (player.repeatMode + 1) % 3
                    }
                    Common.AuroraButton {
                        uiTheme: theme
                        iconOnly: true
                        iconName: "more"
                        text: "Редактировать трек"
                        enabled: player.currentTrackId > 0
                        onClicked: libraryModel.requestEdit(player.currentTrackId)
                    }
                }
                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: (player.streaming ? "ПОТОК  ·  " : "НА ЭТОМ УСТРОЙСТВЕ  ·  ") + (player.currentFormat || "AURORA")
                    color: theme.textMuted
                    font.pixelSize: 8
                    font.letterSpacing: 1.7
                }
                Item {
                    Layout.preferredHeight: root.compact ? 4 : 24
                }
            }
            NowPlayingPanel {
                visible: settingsStore.sideQueueVisible && !root.compact && root.width > 1050
                queueOnly: true
                Layout.preferredWidth: 300
                Layout.preferredHeight: Math.min(500, 170 + player.queue.length * 60)
                Layout.alignment: Qt.AlignVCenter
            }
        }
        NowPlayingPanel {
            visible: root.compact && root.mobileQueue
            queueOnly: true
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
    Common.AuroraButton {
        id: closeButton
        z: 10
        uiTheme: theme
        iconName: "close"
        iconOnly: true
        text: "Выйти из прослушивания · Esc"
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: root.compact ? 16 : 24
        implicitWidth: 44
        implicitHeight: 44
        onClicked: root.closeRequested()
    }
    Common.AuroraButton {
        z: 10
        uiTheme: theme
        iconName: root.mobileQueue ? "music" : "queue"
        iconOnly: true
        text: "Очередь"
        visible: root.compact
        anchors.right: closeButton.left
        anchors.rightMargin: 8
        anchors.verticalCenter: closeButton.verticalCenter
        onClicked: root.mobileQueue = !root.mobileQueue
    }
}
