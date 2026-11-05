// Боковая навигация: разделы, плейлисты, избранное и индикатор подключения NameLess UI.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../common" as Common

Rectangle {
    id: root
    property int currentPage: 0
    property bool collapsed: false
    signal pageSelected(int page)
    signal createPlaylistRequested
    Theme {
        id: theme
    }
    color: theme.sidebar
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.collapsed ? 10 : 20
        spacing: 8
        RowLayout {
            Layout.alignment: root.collapsed ? Qt.AlignHCenter : Qt.AlignLeft
            Layout.topMargin: 12
            Layout.bottomMargin: root.collapsed ? 16 : 28
            spacing: 12
            Rectangle {
                width: 38
                height: 38
                radius: 13
                color: theme.accent
                Text {
                    anchors.centerIn: parent
                    text: "a"
                    color: theme.textStrong
                    font.pixelSize: 34
                    font.weight: Font.DemiBold
                    font.italic: true
                    anchors.verticalCenterOffset: -3
                }
            }
            Column {
                visible: !root.collapsed
                spacing: 3
                Text {
                    text: "aurora"
                    color: theme.text
                    font.pixelSize: 25
                    font.weight: Font.DemiBold
                    font.letterSpacing: -0.8
                }
                Text {
                    text: "NAMELESS MUSIC"
                    color: theme.textMuted
                    font.pixelSize: 8
                    font.letterSpacing: 2
                }
            }
        }
        // Разделы приложения; page — номер вкладки (0 главная, 1 библиотека, 2 импорт, 4 поиск).
        Repeater {
            model: [
                {
                    label: "Главная",
                    icon: "home",
                    page: 0
                },
                {
                    label: "Поиск музыки",
                    icon: "search",
                    page: 4
                },
                {
                    label: "Моя музыка",
                    icon: "library",
                    page: 1
                },
                {
                    label: "Импорт музыки",
                    icon: "download",
                    page: 2
                }
            ]
            delegate: ItemDelegate {
                required property int index
                required property var modelData
                Layout.fillWidth: true
                implicitHeight: 46
                leftPadding: 12
                rightPadding: 12
                Accessible.name: modelData.label
                ToolTip.visible: root.collapsed && hovered
                ToolTip.text: modelData.label
                onClicked: {
                    if (modelData.page === 1)
                        libraryModel.playlistFilter = 0;
                    root.pageSelected(modelData.page);
                }
                background: Rectangle {
                    radius: 10
                    color: root.currentPage === modelData.page ? theme.selected : parent.hovered ? theme.surfaceHigh : "transparent"
                }
                contentItem: Item {
                    Common.AuroraIcon {
                        id: navigationIcon
                        x: root.collapsed ? (parent.width - width) / 2 : 0
                        anchors.verticalCenter: parent.verticalCenter
                        name: modelData.icon
                        size: 20
                        color: root.currentPage === modelData.page ? theme.accent : theme.subtext
                    }
                    Text {
                        anchors.left: navigationIcon.right
                        anchors.leftMargin: 12
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        visible: !root.collapsed
                        text: modelData.label
                        color: root.currentPage === modelData.page ? theme.text : theme.subtext
                        font.pixelSize: 14
                        elide: Text.ElideRight
                    }
                }
            }
        }
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 1
            color: theme.line
            Layout.topMargin: 18
            Layout.bottomMargin: 14
        }
        Item {
            Layout.fillWidth: true
            implicitHeight: 40
            Text {
                visible: !root.collapsed
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                text: "ТВОЯ КОЛЛЕКЦИЯ"
                font.pixelSize: 9
                font.letterSpacing: 1.5
                color: theme.textMuted
            }
            Common.AuroraButton {
                uiTheme: theme
                iconOnly: true
                iconName: "plus"
                text: "Создать плейлист"
                width: 36
                height: 36
                x: root.collapsed ? (parent.width - width) / 2 : parent.width - width
                anchors.verticalCenter: parent.verticalCenter
                onClicked: root.createPlaylistRequested()
            }
        }
        ItemDelegate {
            Layout.fillWidth: true
            implicitHeight: 44
            leftPadding: 12
            rightPadding: 12
            Accessible.name: "Мне нравится"
            ToolTip.visible: root.collapsed && hovered
            ToolTip.text: "Мне нравится"
            // Значение -1 означает специальный фильтр «Мне нравится».
            onClicked: {
                libraryModel.playlistFilter = -1;
                root.pageSelected(1);
            }
            background: Rectangle {
                color: parent.hovered ? theme.surface : "transparent"
                radius: 10
            }
            contentItem: Item {
                Common.AuroraIcon {
                    id: heartIcon
                    x: root.collapsed ? (parent.width - width) / 2 : 0
                    anchors.verticalCenter: parent.verticalCenter
                    name: "heart"
                    size: 20
                    color: theme.accent
                }
                Text {
                    anchors.left: heartIcon.right
                    anchors.leftMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    visible: !root.collapsed
                    text: "Мне нравится"
                    font.pixelSize: 13
                    color: theme.text
                }
            }
        }
        Item {
            visible: root.collapsed
            Layout.fillHeight: true
        }
        ListView {
            visible: !root.collapsed
            Layout.fillHeight: true
            Layout.fillWidth: true
            model: libraryModel.playlists
            clip: true
            spacing: 3
            delegate: ItemDelegate {
                required property var modelData
                width: ListView.view.width
                height: 58
                onClicked: {
                    libraryModel.playlistFilter = modelData.id;
                    root.pageSelected(1);
                }
                background: Rectangle {
                    radius: 10
                    color: libraryModel.playlistFilter === modelData.id && root.currentPage === 1 ? theme.selected : parent.hovered ? theme.surface : "transparent"
                }
                contentItem: RowLayout {
                    spacing: 12
                    Rectangle {
                        width: 34
                        height: 34
                        radius: 7
                        color: theme.surfaceHigh
                        Common.AuroraIcon {
                            anchors.centerIn: parent
                            name: "music"
                            size: 16
                            color: theme.subtext
                        }
                    }
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 5
                        Text {
                            Layout.fillWidth: true
                            text: modelData.name
                            color: theme.text
                            font.pixelSize: 13
                            elide: Text.ElideRight
                        }
                        Text {
                            text: modelData.count + " треков"
                            color: theme.textMuted
                            font.pixelSize: 10
                        }
                    }
                }
            }
        }
        Common.AuroraButton {
            visible: root.collapsed
            uiTheme: theme
            Layout.fillWidth: true
            iconOnly: true
            iconName: "next"
            text: "Развернуть навигацию"
            onClicked: settingsStore.compactNavigation = false
        }
        Common.AuroraButton {
            uiTheme: theme
            Layout.fillWidth: true
            text: "Настройки"
            iconOnly: root.collapsed
            iconName: "settings"
            onClicked: root.pageSelected(3)
        }
        RowLayout {
            Layout.topMargin: 12
            spacing: 7
            Layout.alignment: root.collapsed ? Qt.AlignHCenter : Qt.AlignLeft
            Rectangle {
                width: 5
                height: 5
                radius: 3
                color: namelessBridge.connected ? theme.good : theme.accent
            }
            Text {
                visible: !root.collapsed
                text: namelessBridge.connected ? "В ритме NameLess UI" : "Локально. Всегда рядом."
                font.pixelSize: 10
                color: theme.textMuted
            }
        }
    }
}
