// Сводка о диске: сколько занимает библиотека, остальные файлы и хватит ли места для загрузки.
import QtQuick
import QtQuick.Layouts
import "../common" as Common

Common.AuroraCard {
    id: root
    property var requiredBytes: 0
    readonly property real total: Math.max(1, storageManager.totalBytes)
    // Доли библиотеки и прочих файлов от общего объёма диска.
    readonly property real libraryFraction: Math.min(1, storageManager.libraryBytes / total)
    readonly property real otherFraction: Math.max(0, Math.min(1, (storageManager.totalBytes - storageManager.freeBytes - storageManager.libraryBytes) / total))
    Theme {
        id: theme
    }
    uiTheme: theme
    padding: 20
    implicitHeight: content.implicitHeight + padding * 2
    ColumnLayout {
        id: content
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        spacing: 14
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
                    name: "folder"
                    size: 22
                    color: theme.accent
                }
            }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                Text {
                    text: "Место для музыки"
                    color: theme.text
                    font.pixelSize: 17
                    font.weight: Font.DemiBold
                }
                Text {
                    Layout.fillWidth: true
                    text: storageManager.busy ? "Обновляем размер коллекции…" : storageManager.formatBytes(storageManager.freeBytes) + " свободно на диске"
                    color: theme.subtext
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                }
            }
            Common.AuroraButton {
                uiTheme: theme
                iconName: "refresh"
                iconOnly: true
                text: "Обновить размер"
                enabled: !storageManager.busy
                onClicked: storageManager.refresh()
            }
        }
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 10
            radius: 5
            color: theme.surfaceHigh
            clip: true
            // Полоса библиотеки, за ней примыкает полупрозрачная полоса прочих файлов.
            Rectangle {
                width: Math.max(storageManager.libraryBytes > 0 ? 3 : 0, parent.width * root.libraryFraction)
                height: parent.height
                radius: 5
                color: theme.accent
            }
            Rectangle {
                x: Math.max(storageManager.libraryBytes > 0 ? 3 : 0, parent.width * root.libraryFraction) + 2
                width: Math.max(0, Math.min(parent.width - x, parent.width * root.otherFraction))
                height: parent.height
                radius: 5
                color: Qt.rgba(theme.accent.r, theme.accent.g, theme.accent.b, .23)
            }
        }
        GridLayout {
            Layout.fillWidth: true
            columns: root.width < 470 ? 1 : 3
            rowSpacing: 10
            columnSpacing: 16
            Repeater {
                model: [
                    {
                        label: "Библиотека Aurora",
                        value: storageManager.libraryBytes,
                        tint: theme.accent
                    },
                    {
                        label: "Остальные файлы",
                        value: Math.max(0, storageManager.totalBytes - storageManager.freeBytes - storageManager.libraryBytes),
                        tint: theme.subtext
                    },
                    {
                        label: "Всего на диске",
                        value: storageManager.totalBytes,
                        tint: theme.textMuted
                    }
                ]
                delegate: ColumnLayout {
                    required property var modelData
                    Layout.fillWidth: true
                    spacing: 5
                    Text {
                        text: modelData.label
                        color: modelData.tint
                        font.pixelSize: 10
                    }
                    Text {
                        text: storageManager.formatBytes(modelData.value)
                        color: theme.text
                        font.pixelSize: root.width < 470 ? 16 : 19
                        font.weight: Font.Medium
                    }
                }
            }
        }
        Text {
            Layout.fillWidth: true
            text: storageManager.libraryFolder
            color: theme.textMuted
            font.pixelSize: 10
            elide: Text.ElideMiddle
        }
        Text {
            Layout.fillWidth: true
            visible: root.requiredBytes > 0
            text: root.requiredBytes > storageManager.freeBytes ? "Для этой загрузки не хватает места" : "Загрузка ≈ " + storageManager.formatBytes(root.requiredBytes) + " · останется " + storageManager.formatBytes(Math.max(0, storageManager.freeBytes - root.requiredBytes))
            color: root.requiredBytes > storageManager.freeBytes ? theme.danger : theme.good
            font.pixelSize: 12
            wrapMode: Text.WordWrap
        }
    }
}
