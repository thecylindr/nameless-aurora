import QtQuick
import QtQuick.Layouts
import "../common" as Common

Common.AuroraCard {
    id: root
    property var requiredBytes: 0
    Theme { id: theme }
    uiTheme: theme
    padding: 16
    implicitHeight: content.implicitHeight + padding * 2

    ColumnLayout {
        id: content
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: root.padding
        spacing: 10
        RowLayout {
            Layout.fillWidth: true
            Text { text: "Хранилище"; color: theme.text; font.pixelSize: theme.bodySize; font.weight: Font.Medium; Layout.fillWidth: true }
            Text { text: storageManager.formatBytes(storageManager.freeBytes) + " свободно"; color: theme.good; font.pixelSize: theme.captionSize }
        }
        Common.AuroraProgress {
            uiTheme: theme
            Layout.fillWidth: true
            from: 0
            to: Math.max(1, storageManager.totalBytes)
            value: storageManager.totalBytes - storageManager.freeBytes
        }
        Text {
            Layout.fillWidth: true
            text: root.requiredBytes > 0
                  ? "После импорта останется примерно " + storageManager.formatBytes(Math.max(0, storageManager.freeBytes - root.requiredBytes))
                  : storageManager.formatBytes(storageManager.libraryBytes) + " занято библиотекой · " + storageManager.formatBytes(storageManager.totalBytes) + " всего"
            color: theme.subtext
            font.pixelSize: theme.captionSize
            wrapMode: Text.WordWrap
        }
    }
}
