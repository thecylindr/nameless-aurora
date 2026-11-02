import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import "../common" as Common
import "../components" as Components

Item {
    id: root
    property bool compact: false
    Theme { id: theme }
    Component.onCompleted: storageManager.refresh()

    ScrollView {
        id: scroll
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth
        background: Item {}
        ScrollBar.vertical: Common.AuroraScrollBar { uiTheme: theme; x: scroll.width - width; height: scroll.height }

        ColumnLayout {
            width: Math.min(820, Math.max(280, scroll.availableWidth - (compact ? 32 : 56)))
            x: Math.max(compact ? 16 : 28, (scroll.availableWidth - width) / 2)
            spacing: 18

            Item { implicitHeight: compact ? 16 : 28 }
            Text { text: "Настройки"; color: theme.text; font.pixelSize: compact ? 24 : 30; font.weight: Font.DemiBold }

            Common.AuroraSection {
                uiTheme: theme
                Layout.fillWidth: true
                title: "Библиотека и память"
                description: "Файлы и база остаются на этом устройстве"
                content: ColumnLayout {
                    width: parent.width
                    spacing: 12
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 3
                            Text { text: "Папка музыки"; color: theme.subtext; font.pixelSize: theme.captionSize }
                            Text { Layout.fillWidth: true; text: settingsStore.libraryFolder; color: theme.text; font.pixelSize: theme.bodySize; elide: Text.ElideMiddle }
                        }
                        Common.AuroraButton { uiTheme: theme; text: "Изменить"; iconName: "folder"; onClicked: libraryFolderDialog.open() }
                    }
                    Components.StorageSummary { Layout.fillWidth: true }
                }
            }

            Common.AuroraSection {
                uiTheme: theme
                Layout.fillWidth: true
                title: "Импорт"
                description: "Качество и поведение массовой загрузки"
                content: ColumnLayout {
                    width: parent.width
                    spacing: 16
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 7
                        Text { text: "Качество по умолчанию"; color: theme.subtext; font.pixelSize: theme.captionSize }
                        Common.AuroraSegmented {
                            uiTheme: theme
                            Layout.fillWidth: true
                            model: ["Оригинал", "192 kbps", "128 kbps"]
                            currentIndex: ["original", "balanced", "compact"].indexOf(settingsStore.quality)
                            onOptionSelected: index => settingsStore.quality = ["original", "balanced", "compact"][index]
                        }
                    }
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 7
                        Text { text: "Параллельные загрузки"; color: theme.subtext; font.pixelSize: theme.captionSize }
                        Common.AuroraSegmented {
                            uiTheme: theme
                            Layout.fillWidth: true
                            model: ["1", "2", "3", "4"]
                            currentIndex: settingsStore.downloadConcurrency - 1
                            onOptionSelected: index => settingsStore.downloadConcurrency = index + 1
                        }
                    }
                    Common.AuroraSwitch {
                        uiTheme: theme
                        Layout.fillWidth: true
                        text: "Подтверждать массовую загрузку"
                        subtext: "Показывать число треков и оценку памяти до старта"
                        checked: settingsStore.confirmBulkDownload
                        onToggled: settingsStore.confirmBulkDownload = checked
                    }
                }
            }

            Common.AuroraSection {
                uiTheme: theme
                Layout.fillWidth: true
                title: "Браузерный профиль"
                description: "Нужен только для авторизованного импорта; пароли и cookies не сохраняются"
                content: ColumnLayout {
                    width: parent.width
                    spacing: 12
                    Common.AuroraField {
                        uiTheme: theme
                        Layout.fillWidth: true
                        placeholderText: "firefox, chromium, chrome"
                        text: settingsStore.browser
                        onEditingFinished: settingsStore.browser = text
                    }
                    Common.AuroraField {
                        uiTheme: theme
                        Layout.fillWidth: true
                        placeholderText: "Профиль, например Default (необязательно)"
                        text: settingsStore.browserProfile
                        onEditingFinished: settingsStore.browserProfile = text
                    }
                }
            }

            Components.DependencyCard { Layout.fillWidth: true }

            Common.AuroraButton {
                uiTheme: theme
                danger: true
                text: "Сбросить настройки импорта"
                iconName: "refresh"
                onClicked: settingsStore.resetPreferences()
            }
            Item { implicitHeight: 24 }
        }
    }

    FolderDialog {
        id: libraryFolderDialog
        title: "Папка библиотеки"
        currentFolder: encodeURI("file://" + settingsStore.libraryFolder)
        onAccepted: {
            settingsStore.libraryFolder = selectedFolder
            storageManager.refresh()
        }
    }
}
