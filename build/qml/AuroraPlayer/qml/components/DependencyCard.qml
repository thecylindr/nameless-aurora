import QtQuick
import QtQuick.Layouts
import "../common" as Common

Common.AuroraCard {
    id: root
    Theme { id: theme }
    uiTheme: theme
    padding: 18
    implicitHeight: content.implicitHeight + padding * 2
    Component.onCompleted: downloadDependencies.refresh()

    ColumnLayout {
        id: content
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: root.padding
        spacing: 12
        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 3
                Text { text: "Загрузчики"; color: theme.text; font.pixelSize: theme.bodySize; font.weight: Font.Medium }
                Text { text: downloadDependencies.runtimeReady ? "Изолированное окружение готово" : "Установятся в папку данных Aurora"; color: downloadDependencies.runtimeReady ? theme.good : theme.subtext; font.pixelSize: theme.captionSize }
            }
            Common.AuroraButton {
                uiTheme: theme
                primary: !downloadDependencies.runtimeReady
                text: downloadDependencies.busy ? "Отмена" : downloadDependencies.runtimeReady ? "Обновить" : "Установить"
                iconName: downloadDependencies.busy ? "close" : "download"
                onClicked: downloadDependencies.busy ? downloadDependencies.cancel() : downloadDependencies.installOrUpdate()
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
            text: downloadDependencies.lastError.length ? downloadDependencies.lastError : downloadDependencies.statusText
            color: downloadDependencies.lastError.length ? theme.danger : theme.subtext
            font.pixelSize: theme.captionSize
            wrapMode: Text.WordWrap
        }
        Text {
            Layout.fillWidth: true
            text: "yt-dlp: " + (downloadDependencies.ytDlpVersion || "не найден") + "\nSpotDL: " + (downloadDependencies.spotdlVersion || "не найден") + "\nFFmpeg: " + (downloadDependencies.ffmpegVersion || "не найден")
            color: theme.subtext
            font.pixelSize: 10
            wrapMode: Text.WrapAnywhere
        }
        Text {
            Layout.fillWidth: true
            text: downloadDependencies.toolsDir
            color: theme.textMuted
            font.pixelSize: 9
            elide: Text.ElideMiddle
        }
    }
}
