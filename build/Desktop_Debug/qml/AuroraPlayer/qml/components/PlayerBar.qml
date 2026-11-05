// Нижняя панель плеера: обложка, управление, перемотка, громкость и таймер сна.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../common" as Common

Item {
    id: root
    property bool compact: false
    property int sleepMinutes: 0
    property real previousVolume: .75
    readonly property color trackAccent: player.hasTrack ? player.atmosphereColor : theme.accent
    // По яркости акцента (взвешенная сумма каналов) выбираем тёмный или светлый цвет иконки.
    readonly property color accentInk: trackAccent.r * .2126 + trackAccent.g * .7152 + trackAccent.b * .0722 > .42 ? "#10151c" : "#ffffff"
    signal expandRequested
    signal queueRequested
    signal equalizerRequested
    implicitHeight: compact ? 96 : settingsStore.compactPlayer ? 92 : 112
    Theme {
        id: theme
    }
    // Форматирует миллисекунды в «м:сс».
    function time(ms) {
        return Math.floor(ms / 60000) + ":" + String(Math.floor(ms / 1000) % 60).padStart(2, "0");
    }
    // Таймер сна: раз в минуту уменьшает счётчик и ставит паузу по достижении нуля.
    Timer {
        interval: 60000
        repeat: true
        running: root.sleepMinutes > 0
        onTriggered: {
            if (--root.sleepMinutes === 0)
                player.pause();
        }
    }
    Rectangle {
        id: surface
        anchors.fill: parent
        anchors.margins: compact ? 6 : 12
        anchors.topMargin: 0
        radius: compact ? 18 : 22
        color: theme.surface
        border.color: Qt.rgba(root.trackAccent.r, root.trackAccent.g, root.trackAccent.b, .13)
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop {
                position: 0
                color: Qt.rgba(theme.surface.r * .85 + root.trackAccent.r * .15, theme.surface.g * .85 + root.trackAccent.g * .15, theme.surface.b * .85 + root.trackAccent.b * .15, 1)
            }
            GradientStop {
                position: .42
                color: theme.surface
            }
            GradientStop {
                position: 1
                color: theme.surface
            }
        }
        RowLayout {
            anchors.fill: parent
            anchors.margins: root.compact ? 12 : settingsStore.compactPlayer ? 8 : 14
            spacing: root.compact ? 10 : settingsStore.compactPlayer ? 14 : 22
            anchors.bottomMargin: root.compact ? 26 : settingsStore.compactPlayer ? 8 : 14
            RowLayout {
                Layout.preferredWidth: root.compact ? -1 : Math.min(300, root.width * .24)
                Layout.maximumWidth: root.compact ? Infinity : Math.min(300, root.width * .24)
                Layout.fillWidth: root.compact
                spacing: 12
                AlbumArt {
                    Layout.preferredWidth: root.compact || settingsStore.compactPlayer ? 46 : 56
                    Layout.preferredHeight: width
                    path: player.currentCoverPath
                    seed: player.currentTitle || "Aurora"
                    cornerRadius: 12
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.expandRequested()
                    }
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 5
                    Text {
                        Layout.fillWidth: true
                        text: player.currentTitle || "Музыка рядом"
                        color: theme.text
                        font.pixelSize: root.compact ? 12 : 14
                        font.weight: Font.DemiBold
                        elide: Text.ElideRight
                    }
                    Text {
                        Layout.fillWidth: true
                        text: player.currentArtist || "Выбери свой первый трек"
                        color: theme.subtext
                        font.pixelSize: 11
                        elide: Text.ElideRight
                    }
                    Row {
                        visible: !root.compact && !settingsStore.compactPlayer && player.hasTrack
                        spacing: 6
                        Rectangle {
                            width: 5
                            height: 5
                            radius: 3
                            anchors.verticalCenter: parent.verticalCenter
                            color: root.trackAccent
                            opacity: player.playing ? .5 + player.audioEnergy * .5 : .35
                        }
                        Text {
                            text: (player.currentFormat || "АУДИО") + (player.equalizer.enabled ? " · EQ" : player.streaming ? "" : " · Локальный файл")
                            color: theme.textMuted
                            font.pixelSize: 9
                        }
                    }
                    TapHandler {
                        acceptedButtons: Qt.LeftButton
                        onTapped: root.expandRequested()
                    }
                    HoverHandler {
                        cursorShape: Qt.PointingHandCursor
                    }
                }
                Common.AuroraButton {
                    uiTheme: theme
                    iconName: "heart"
                    iconOnly: true
                    text: "Мне нравится"
                    primary: player.favorite
                    implicitWidth: 30
                    implicitHeight: 30
                    visible: !root.compact && root.width > 1050
                    enabled: player.currentTrackId > 0
                    onClicked: player.toggleFavorite()
                }
            }
            Item {
                Layout.fillWidth: !root.compact
                Layout.preferredWidth: root.compact ? 112 : 500
                Layout.fillHeight: true
                ColumnLayout {
                    width: Math.min(parent.width, 660)
                    anchors.centerIn: parent
                    spacing: 4
                    RowLayout {
                        Layout.alignment: Qt.AlignHCenter
                        spacing: root.compact ? 2 : 12
                        Common.AuroraButton {
                            uiTheme: theme
                            iconName: "shuffle"
                            iconOnly: true
                            text: "Перемешать"
                            primary: player.shuffle
                            visible: !root.compact
                            implicitWidth: 30
                            implicitHeight: 30
                            onClicked: player.shuffle = !player.shuffle
                        }
                        Common.AuroraButton {
                            uiTheme: theme
                            iconName: "previous"
                            iconOnly: true
                            text: "Назад"
                            visible: !root.compact
                            implicitWidth: 32
                            implicitHeight: 32
                            onClicked: player.previous()
                        }
                        Common.AuroraButton {
                            id: playButton
                            uiTheme: theme
                            iconName: player.playing ? "pause" : "play"
                            iconOnly: true
                            text: player.playing ? "Пауза" : "Слушать"
                            primary: true
                            implicitWidth: root.compact ? 42 : settingsStore.compactPlayer ? 38 : 46
                            implicitHeight: implicitWidth
                            onClicked: player.toggle()
                            background: Rectangle {
                                radius: width / 2
                                color: playButton.down ? Qt.darker(root.trackAccent, 1.15) : playButton.hovered ? Qt.lighter(root.trackAccent, 1.12) : root.trackAccent
                                border.width: playButton.activeFocus ? 2 : 0
                                border.color: theme.text
                            }
                            contentItem: Common.AuroraIcon {
                                name: playButton.iconName
                                size: 20
                                color: root.accentInk
                            }
                        }
                        Common.AuroraButton {
                            uiTheme: theme
                            iconName: "next"
                            iconOnly: true
                            text: "Дальше"
                            implicitWidth: 32
                            implicitHeight: 32
                            onClicked: player.next()
                        }
                        Common.AuroraButton {
                            uiTheme: theme
                            iconName: "more"
                            iconOnly: true
                            text: "Ещё"
                            visible: root.compact
                            implicitWidth: 28
                            implicitHeight: 30
                            onClicked: more.popup()
                        }
                        Common.AuroraButton {
                            uiTheme: theme
                            iconName: "repeat"
                            iconOnly: true
                            text: ["Без повтора", "Повтор очереди", "Повтор трека"][player.repeatMode]
                            primary: player.repeatMode > 0
                            visible: !root.compact
                            implicitWidth: 30
                            implicitHeight: 30
                            onClicked: player.repeatMode = (player.repeatMode + 1) % 3
                            Text {
                                anchors.right: parent.right
                                anchors.top: parent.top
                                text: "1"
                                font.pixelSize: 9
                                color: theme.text
                                visible: player.repeatMode === 2
                            }
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        visible: !root.compact
                        spacing: 10
                        Text {
                            Layout.preferredWidth: 32
                            text: root.time(player.position)
                            color: theme.textMuted
                            font.pixelSize: 10
                        }
                        Common.AuroraSlider {
                            uiTheme: theme
                            Layout.fillWidth: true
                            implicitHeight: 20
                            from: 0
                            to: Math.max(1, player.duration)
                            value: player.position
                            enabled: player.hasTrack && player.seekable
                            onMoved: player.seek(value)
                        }
                        Text {
                            Layout.preferredWidth: 32
                            text: root.time(player.duration)
                            color: theme.textMuted
                            font.pixelSize: 10
                            horizontalAlignment: Text.AlignRight
                        }
                    }
                }
            }
            RowLayout {
                visible: !root.compact
                Layout.preferredWidth: root.width > 1120 ? 314 : 224
                Layout.maximumWidth: Layout.preferredWidth
                spacing: 6
                Common.AuroraButton {
                    uiTheme: theme
                    iconName: "equalizer"
                    iconOnly: true
                    text: "Эквалайзер"
                    primary: player.equalizer.enabled
                    implicitWidth: 32
                    implicitHeight: 32
                    onClicked: root.equalizerRequested()
                }
                Common.AuroraButton {
                    uiTheme: theme
                    iconName: player.volume > 0 ? "volume" : "muted"
                    iconOnly: true
                    text: player.volume > 0 ? "Выключить звук" : "Включить звук"
                    implicitWidth: 28
                    implicitHeight: 30
                    // Запоминаем прежнюю громкость, чтобы кнопка mute могла её вернуть.
                    onClicked: {
                        if (player.volume > 0) {
                            root.previousVolume = player.volume;
                            player.volume = 0;
                        } else
                            player.volume = root.previousVolume;
                    }
                }
                Common.AuroraSlider {
                    uiTheme: theme
                    Layout.preferredWidth: root.width > 1120 ? 96 : 68
                    Layout.maximumWidth: Layout.preferredWidth
                    from: 0
                    to: 1
                    value: player.volume
                    onMoved: player.volume = value
                }
                Text {
                    visible: root.width > 1120
                    Layout.preferredWidth: 30
                    text: Math.round(player.volume * 100) + "%"
                    color: theme.subtext
                    font.pixelSize: 10
                }
                Rectangle {
                    Layout.preferredWidth: 1
                    Layout.preferredHeight: 22
                    color: theme.line
                }
                Common.AuroraButton {
                    uiTheme: theme
                    iconName: "queue"
                    iconOnly: true
                    text: "Очередь воспроизведения"
                    implicitWidth: 30
                    implicitHeight: 30
                    onClicked: root.queueRequested()
                }
                Common.AuroraButton {
                    uiTheme: theme
                    iconName: "expand"
                    iconOnly: true
                    text: "Погрузиться в музыку"
                    implicitWidth: 30
                    implicitHeight: 30
                    visible: root.width > 1120
                    onClicked: root.expandRequested()
                }
                Common.AuroraButton {
                    uiTheme: theme
                    iconName: "more"
                    iconOnly: true
                    text: "Ещё"
                    implicitWidth: 30
                    implicitHeight: 30
                    onClicked: more.popup()
                }
            }
        }
        Common.AuroraSlider {
            uiTheme: theme
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 9
            visible: root.compact
            implicitHeight: 16
            from: 0
            to: Math.max(1, player.duration)
            value: player.position
            enabled: player.hasTrack && player.seekable
            onMoved: player.seek(value)
        }
    }
    Common.AuroraMenu {
        id: more
        uiTheme: theme
        Common.AuroraMenuItem {
            uiTheme: theme
            text: "Очередь воспроизведения"
            onTriggered: root.queueRequested()
        }
        Common.AuroraMenuItem {
            uiTheme: theme
            text: "Эквалайзер…"
            onTriggered: root.equalizerRequested()
        }
        Common.AuroraMenuItem {
            uiTheme: theme
            text: "Таймер · 15 минут"
            onTriggered: root.sleepMinutes = 15
        }
        Common.AuroraMenuItem {
            uiTheme: theme
            text: "Таймер · 30 минут"
            onTriggered: root.sleepMinutes = 30
        }
        Common.AuroraMenuItem {
            uiTheme: theme
            text: "Таймер · 60 минут"
            onTriggered: root.sleepMinutes = 60
        }
        Common.AuroraMenuItem {
            uiTheme: theme
            text: "Выключить таймер"
            enabled: root.sleepMinutes > 0
            onTriggered: root.sleepMinutes = 0
        }
    }
}
