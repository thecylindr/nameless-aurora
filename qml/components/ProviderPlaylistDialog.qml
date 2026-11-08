import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ".."
import "../common" as Common

Dialog {
    id: root

    property string provider: "yandex"
    readonly property var tracks: provider === "vk" ? homeFeed.vkTracks : homeFeed.yandexTracks
    readonly property bool busy: provider === "vk" ? homeFeed.vkBusy : homeFeed.busy
    readonly property string providerName: provider === "vk" ? "VK Музыка" : "Яндекс Музыка"
    readonly property string status: provider === "vk" ? homeFeed.vkStatus : homeFeed.status
    property int pageIndex: 0
    property int pageSize: 12
    readonly property int pageCount: Math.max(1, Math.ceil(tracks.length / pageSize))
    readonly property var pageTracks: tracks.slice(pageIndex * pageSize, (pageIndex + 1) * pageSize)
    readonly property bool narrow: width < 560

    function play(index) {
        if (provider === "vk")
            homeFeed.playVk(index);
        else
            homeFeed.playYandex(index);
    }

    implicitWidth: 820
    implicitHeight: 800
    modal: true
    focus: true
    width: Math.min(820, Math.max(0, parent.width - 32))
    height: Math.min(800, Math.max(0, parent.height - 32))
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    padding: narrow ? 16 : 24
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    onPageCountChanged: pageIndex = Math.min(pageIndex, pageCount - 1)
    onPageSizeChanged: pageIndex = 0
    onPageIndexChanged: list.positionViewAtBeginning()
    onOpened: pageIndex = 0

    Theme {
        id: theme
    }

    background: Rectangle {
        color: theme.surface
        border.color: theme.line
        radius: 24
    }

    Overlay.modal: Rectangle {
        color: "#b006080f"
    }

    contentItem: ColumnLayout {
        spacing: 14

        RowLayout {
            Layout.fillWidth: true

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 6

                Text {
                    text: root.providerName.toUpperCase()
                    color: theme.accent
                    font.pixelSize: 10
                    font.letterSpacing: 1.4
                }

                Text {
                    text: "Рекомендации для тебя"
                    color: theme.text
                    font.pixelSize: root.narrow ? 21 : 27
                    font.weight: Font.DemiBold
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                }

            }

            Common.AuroraButton {
                uiTheme: theme
                iconOnly: true
                iconName: "close"
                text: "Закрыть плейлист"
                onClicked: root.close()
            }

        }

        Text {
            Layout.fillWidth: true
            text: root.status + (root.tracks.length ? " · " + root.tracks.length + " полученных записей" : "")
            color: theme.subtext
            font.pixelSize: 12
            wrapMode: Text.WordWrap
        }

        RowLayout {
            Layout.fillWidth: true

            Common.AuroraButton {
                uiTheme: theme
                primary: true
                iconName: "play"
                text: "Слушать всё"
                enabled: root.tracks.length > 0
                onClicked: root.play(0)
            }

            Common.AuroraButton {
                uiTheme: theme
                iconName: "refresh"
                iconOnly: root.narrow
                text: "Обновить подборку"
                enabled: !root.busy
                onClicked: root.provider === "vk" ? homeFeed.refreshVk(true) : homeFeed.refreshYandex(true)
            }

            Item {
                Layout.fillWidth: true
            }

        }

        Text {
            Layout.fillWidth: true
            visible: homeFeed.playbackStatus.length > 0
            text: homeFeed.playbackStatus
            color: theme.accent
            font.pixelSize: 12
            wrapMode: Text.WordWrap
        }

        Common.AuroraProgress {
            uiTheme: theme
            Layout.fillWidth: true
            visible: root.busy
            indeterminate: true
        }

        ListView {
            id: list

            Layout.fillWidth: true
            Layout.fillHeight: true
            model: root.pageTracks
            spacing: 6
            clip: true
            boundsBehavior: Flickable.StopAtBounds

            Text {
                anchors.centerIn: parent
                width: parent.width - 24
                visible: !root.tracks.length
                text: root.busy ? "Собираем плейлист…" : "Сервис пока не вернул записей. Обнови подборку или проверь подключение аккаунта."
                color: theme.subtext
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
            }

            ScrollBar.vertical: Common.AuroraScrollBar {
                uiTheme: theme
            }

            delegate: Rectangle {
                id: row

                required property var modelData
                required property int index
                readonly property int position: root.pageIndex * root.pageSize + index
                readonly property string key: modelData.key || modelData.url || ""
                readonly property bool pending: homeFeed.resolvingUrl === key || (player.streamKey === key && player.streamLoading)
                readonly property bool current: player.streamKey === key && key.length > 0

                width: list.width - 10
                height: 76
                radius: 13
                color: current ? theme.selected : theme.surfaceHigh

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 12

                    Text {
                        visible: !root.narrow
                        text: row.position + 1
                        color: theme.textMuted
                        font.pixelSize: 11
                        Layout.preferredWidth: 24
                        horizontalAlignment: Text.AlignHCenter
                    }

                    AlbumArt {
                        Layout.preferredWidth: 46
                        Layout.preferredHeight: 46
                        path: row.modelData.coverPath || row.modelData.coverUrl || ""
                        seed: row.modelData.title || ""
                        cornerRadius: 9
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 5

                        Text {
                            Layout.fillWidth: true
                            text: row.modelData.title
                            color: theme.text
                            font.pixelSize: 13
                            font.weight: Font.Medium
                            elide: Text.ElideRight
                        }

                        Text {
                            Layout.fillWidth: true
                            text: row.modelData.artist || "Неизвестный исполнитель"
                            color: theme.subtext
                            font.pixelSize: 11
                            elide: Text.ElideRight
                        }

                    }

                    Text {
                        visible: !root.narrow
                        text: row.modelData.durationMs > 0 ? Math.floor(row.modelData.durationMs / 60000) + ":" + String(Math.floor(row.modelData.durationMs / 1000) % 60).padStart(2, "0") : "—"
                        color: theme.subtext
                        font.pixelSize: 11
                    }

                    Common.AuroraButton {
                        uiTheme: theme
                        iconOnly: true
                        implicitWidth: 40
                        implicitHeight: 40
                        primary: row.current
                        iconName: row.pending ? "" : row.current && player.playing ? "pause" : "play"
                        text: row.pending ? "Подключаем аудио…" : row.current && player.playing ? "Пауза" : "Слушать онлайн"
                        enabled: !row.pending
                        onClicked: row.current ? player.toggle() : root.play(row.position)

                        Common.ProgressRing {
                            anchors.centerIn: parent
                            width: 22
                            height: 22
                            visible: row.pending
                            indeterminate: true
                            showLabel: false
                            strokeWidth: 2
                            color: theme.accent
                        }

                    }

                }

            }

        }

        RowLayout {
            Layout.fillWidth: true

            Text {
                Layout.fillWidth: true
                text: root.tracks.length ? (root.pageIndex * root.pageSize + 1) + "–" + Math.min((root.pageIndex + 1) * root.pageSize, root.tracks.length) + " из " + root.tracks.length : "Нет записей"
                color: theme.subtext
                font.pixelSize: 11
            }

            Common.AuroraCombo {
                visible: !root.narrow
                uiTheme: theme
                Layout.preferredWidth: 140
                model: ["12 на странице", "24 на странице", "48 на странице"]
                onActivated: root.pageSize = [12, 24, 48][currentIndex]
            }

            Common.AuroraButton {
                uiTheme: theme
                iconOnly: true
                iconName: "previous"
                text: "Предыдущая страница"
                implicitHeight: 34
                enabled: root.pageIndex > 0
                onClicked: root.pageIndex--
            }

            Text {
                text: (root.pageIndex + 1) + " / " + root.pageCount
                color: theme.text
                font.pixelSize: 12
            }

            Common.AuroraButton {
                uiTheme: theme
                iconOnly: true
                iconName: "next"
                text: "Следующая страница"
                implicitHeight: 34
                enabled: root.pageIndex + 1 < root.pageCount
                onClicked: root.pageIndex++
            }

        }

        Text {
            Layout.fillWidth: true
            text: "В очереди остаётся вся полученная подборка. Переключение страниц не прерывает музыку и ничего не скачивает."
            color: theme.textMuted
            font.pixelSize: 10
            wrapMode: Text.WordWrap
        }

    }

}
