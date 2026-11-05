// Карточка состояния движка загрузки: версии yt-dlp, FFmpeg и SpotDL, установка и диагностика.
import QtQuick
import QtQuick.Layouts
import "../common" as Common

Common.AuroraCard {
    id: root
    property bool detailsOpen: false
    Theme {
        id: theme
    }
    uiTheme: theme
    padding: 20
    implicitHeight: content.implicitHeight + padding * 2
    // Извлекает номер версии из вывода утилиты; при отсутствии — обрезает строку.
    function version(value) {
        if (!value)
            return "Не установлен";
        const match = value.match(/(?:version\s+)?(n?\d[\w.\-]*)/);
        return match ? match[1] : value.slice(0, 32);
    }
    ColumnLayout {
        id: content
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        spacing: 16
        RowLayout {
            Layout.fillWidth: true
            spacing: 12
            Rectangle {
                Layout.preferredWidth: 42
                Layout.preferredHeight: 42
                radius: 13
                color: theme.selected
                Common.AuroraIcon {
                    anchors.centerIn: parent
                    name: "download"
                    size: 22
                    color: theme.accent
                }
            }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                Text {
                    text: "Движок загрузки"
                    color: theme.text
                    font.pixelSize: 17
                    font.weight: Font.DemiBold
                }
                Text {
                    Layout.fillWidth: true
                    text: downloadDependencies.busy ? "Подготавливаем инструменты…" : downloadDependencies.runtimeReady ? "Готов к поиску и загрузке" : "Нужна подготовка"
                    color: downloadDependencies.runtimeReady ? theme.good : theme.subtext
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                }
            }
        }
        GridLayout {
            Layout.fillWidth: true
            columns: root.width < 490 ? 1 : 3
            rowSpacing: 8
            columnSpacing: 8
            Repeater {
                model: [
                    {
                        name: "YouTube и сайты",
                        version: downloadDependencies.ytDlpVersion,
                        engine: "yt-dlp"
                    },
                    {
                        name: "Обработка аудио",
                        version: downloadDependencies.ffmpegVersion,
                        engine: "FFmpeg"
                    },
                    {
                        name: "Spotify · дополнение",
                        version: downloadDependencies.spotdlVersion,
                        engine: "SpotDL"
                    }
                ]
                delegate: Rectangle {
                    required property var modelData
                    Layout.fillWidth: true
                    implicitHeight: 72
                    radius: 12
                    color: theme.surfaceHigh
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 5
                        Text {
                            text: modelData.name
                            color: theme.subtext
                            font.pixelSize: 10
                        }
                        Text {
                            Layout.fillWidth: true
                            text: modelData.engine + " · " + root.version(modelData.version)
                            color: modelData.version ? theme.text : theme.textMuted
                            font.pixelSize: 12
                            elide: Text.ElideRight
                        }
                    }
                }
            }
        }
        Common.AuroraProgress {
            uiTheme: theme
            Layout.fillWidth: true
            visible: downloadDependencies.busy
            indeterminate: true
        }
        Text {
            Layout.fillWidth: true
            text: downloadDependencies.lastError || downloadDependencies.statusText
            color: downloadDependencies.lastError ? theme.danger : theme.subtext
            font.pixelSize: 12
            wrapMode: Text.WordWrap
            lineHeight: 1.3
            visible: text.length > 0
        }
        Flow {
            Layout.fillWidth: true
            spacing: 8
            Common.AuroraButton {
                uiTheme: theme
                primary: !downloadDependencies.runtimeReady
                text: downloadDependencies.busy ? "Отменить" : downloadDependencies.runtimeReady ? "Обновить инструменты" : "Подготовить загрузку"
                iconName: downloadDependencies.busy ? "close" : "download"
                onClicked: downloadDependencies.busy ? downloadDependencies.cancel() : downloadDependencies.installOrUpdate()
            }
            Common.AuroraButton {
                uiTheme: theme
                iconName: "refresh"
                text: "Проверить"
                enabled: !downloadDependencies.busy
                onClicked: downloadDependencies.refresh()
            }
            Common.AuroraButton {
                uiTheme: theme
                text: root.detailsOpen ? "Меньше деталей" : "Подробнее"
                onClicked: root.detailsOpen = !root.detailsOpen
            }
        }
        Text {
            Layout.fillWidth: true
            visible: root.detailsOpen
            text: "Инструменты установлены отдельно от системы.\n" + downloadDependencies.toolsDir + "\n\n" + (downloadDependencies.ffmpegVersion || "FFmpeg не установлен")
            color: theme.textMuted
            font.pixelSize: 11
            wrapMode: Text.WrapAnywhere
            lineHeight: 1.4
        }
    }
}
