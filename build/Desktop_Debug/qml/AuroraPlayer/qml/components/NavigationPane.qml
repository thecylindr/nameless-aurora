import QtQuick
import QtQuick.Layouts
import "../common" as Common

Rectangle {
    id: root
    property int currentPage: 0
    signal pageSelected(int page)
    Theme { id: theme }
    color: theme.sidebar
    border.color: theme.line
    border.width: 1

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 18
        spacing: 8
        RowLayout {
            Layout.bottomMargin: 24
            spacing: 12
            Rectangle {
                width: 42
                height: 42
                radius: 14
                gradient: Gradient {
                    GradientStop { position: 0; color: theme.accent }
                    GradientStop { position: 1; color: theme.accent2 }
                }
                Text { anchors.centerIn: parent; text: "A"; color: "white"; font.pixelSize: 22; font.bold: true }
            }
            Column {
                Text { text: "AURORA"; color: theme.text; font.pixelSize: 18; font.bold: true }
                Text { text: "YOUR MUSIC SPACE"; color: theme.subtext; font.pixelSize: 8 }
            }
        }
        Repeater {
            model: [
                { label: "Главная", icon: "music" },
                { label: "Библиотека", icon: "library" },
                { label: "Импорт", icon: "download" },
                { label: "Настройки", icon: "settings" }
            ]
            delegate: Common.AuroraButton {
                required property int index
                required property var modelData
                uiTheme: theme
                Layout.fillWidth: true
                text: modelData.label
                iconName: modelData.icon
                primary: root.currentPage === index
                onClicked: root.pageSelected(index)
            }
        }
        Item { Layout.fillHeight: true }
        Common.AuroraCard {
            uiTheme: theme
            Layout.fillWidth: true
            padding: 14
            implicitHeight: 92
            ColumnLayout {
                anchors.fill: parent
                spacing: 5
                Text { text: "Локальная библиотека"; color: theme.text; font.pixelSize: theme.captionSize }
                Text { text: libraryModel.count + " треков"; color: theme.accent; font.pixelSize: 18; font.weight: Font.DemiBold }
                Text { Layout.fillWidth: true; text: storageManager.formatBytes(storageManager.libraryBytes) + " на диске"; color: theme.subtext; font.pixelSize: 10; elide: Text.ElideRight }
            }
        }
    }
}
