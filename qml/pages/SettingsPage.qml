// Страница настроек: оформление, звук, библиотека и музыкальный вкус.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../common" as Common
import "../components" as Components

Item {
    id: root
    property bool compact: false
    property int category: 0
    Theme {
        id: theme
    }
    ScrollView {
        id: scroll
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth
        background: Item {}
        ScrollBar.vertical: Common.AuroraScrollBar {
            uiTheme: theme
            x: scroll.width - width
            height: scroll.height
        }
        ColumnLayout {
            width: Math.min(900, Math.max(280, scroll.availableWidth - (root.compact ? 32 : 64)))
            x: Math.max(root.compact ? 16 : 32, (scroll.availableWidth - width) / 2)
            spacing: 22
            Item {
                implicitHeight: root.compact ? 16 : 28
            }
            Text {
                text: "Настройки"
                color: theme.text
                font.pixelSize: root.compact ? 24 : 30
                font.weight: Font.DemiBold
            }
            Common.AuroraSegmented {
                uiTheme: theme
                Layout.fillWidth: true
                model: ["Оформление", "Звук", "Библиотека", "Мой вкус"]
                currentIndex: root.category
                onOptionSelected: index => {
                    root.category = index;
                    scroll.contentItem.contentY = 0;
                }
            }
            Common.AuroraSection {
                uiTheme: theme
                Layout.fillWidth: true
                visible: root.category === 0
                title: "Атмосфера и оформление"
                description: "Цвет, движение и отклик на музыку"
                content: Components.AppearancePanel {
                    width: parent.width
                }
            }
            Common.AuroraSection {
                uiTheme: theme
                Layout.fillWidth: true
                visible: root.category === 0
                title: "NameLess UI"
                description: namelessBridge.connected ? "Акцент из темы «" + namelessBridge.themeName + "». Изменения применяются автоматически." : "При наличии NameLess UI акцент подхватывается автоматически."
                content: ColumnLayout {
                    width: parent.width
                    spacing: 10
                    Text {
                        Layout.fillWidth: true
                        text: "Медиавиджет и мультимедийные клавиши работают через MPRIS на Linux."
                        color: theme.subtext
                        font.pixelSize: 12
                        wrapMode: Text.WordWrap
                    }
                    Text {
                        Layout.fillWidth: true
                        text: "Пробел — пауза · Ctrl+← / → — трек\nCtrl+O — файлы · Ctrl+F — поиск\nCtrl+N — плейлист · Ctrl+L — экран прослушивания"
                        color: theme.subtext
                        font.pixelSize: 12
                        wrapMode: Text.WordWrap
                        lineHeight: 1.5
                    }
                }
            }
            Common.AuroraSection {
                uiTheme: theme
                Layout.fillWidth: true
                visible: root.category === 1
                content: Components.EqualizerPanel {
                    width: parent.width
                }
            }
            Loader {
                Layout.fillWidth: true
                Layout.preferredHeight: item ? item.implicitHeight : 0
                active: root.category === 2
                visible: active
                sourceComponent: Components.LibrarySettings {
                    compact: root.compact
                }
            }
            Common.AuroraSection {
                uiTheme: theme
                Layout.fillWidth: true
                visible: root.category === 3
                title: "Музыка за пределами библиотеки"
                description: "Источники для поиска и подборок по твоему вкусу"
                content: ColumnLayout {
                    width: parent.width
                    spacing: 14
                    Common.AuroraField {
                        uiTheme: theme
                        Layout.fillWidth: true
                        text: musicSearch.taste
                        placeholderText: "Жанры, исполнители, настроение"
                        onEditingFinished: musicSearch.taste = text
                    }
                    Repeater {
                        model: ["YouTube", "Zaycev.net", "VK"]
                        delegate: Common.AuroraSwitch {
                            required property string modelData
                            uiTheme: theme
                            Layout.fillWidth: true
                            text: modelData
                            subtext: modelData === "VK" ? "Поиск и доступный аккаунту раздел рекомендаций" : "Поиск по названию и твоим предпочтениям"
                            checked: musicSearch.enabledSources.indexOf(modelData) >= 0
                            onToggled: {
                                let sources = musicSearch.enabledSources.slice();
                                const i = sources.indexOf(modelData);
                                if (checked && i < 0)
                                    sources.push(modelData);
                                if (!checked && i >= 0)
                                    sources.splice(i, 1);
                                musicSearch.enabledSources = sources;
                            }
                        }
                    }
                    Text {
                        Layout.fillWidth: true
                        text: "Подборки обновляются при нажатии «По моему вкусу». Aurora не запрашивает сервисы в фоне при запуске. Если вкус не заполнен, используется исполнитель из твоей локальной подборки."
                        color: theme.subtext
                        font.pixelSize: 12
                        wrapMode: Text.WordWrap
                        lineHeight: 1.4
                    }
                }
            }
            Item {
                implicitHeight: 28
            }
        }
    }
}
