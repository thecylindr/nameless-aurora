import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import "../common" as Common
import "../components" as Components

Item {
    id: root
    property bool compact: false
    signal importUrlRequested()
    Theme { id: theme }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: compact ? 16 : 28
        spacing: 18

        RowLayout {
            Layout.fillWidth: true
            spacing: 12
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 3
                Text { text: "Библиотека"; color: theme.text; font.pixelSize: compact ? 24 : 30; font.weight: Font.DemiBold }
                Text { text: libraryModel.count + " треков · " + storageManager.formatBytes(storageManager.libraryBytes); color: theme.subtext; font.pixelSize: theme.captionSize }
            }
            Common.AuroraButton {
                uiTheme: theme
                iconName: "download"
                text: compact ? "" : "Импорт"
                iconOnly: compact
                onClicked: root.importUrlRequested()
            }
        }

        Common.AuroraField {
            id: searchField
            uiTheme: theme
            Layout.fillWidth: true
            placeholderText: "Название, исполнитель или альбом"
            iconName: "search"
            text: libraryModel.searchText
            onTextChanged: libraryModel.searchText = text
        }

        Common.AuroraCard {
            uiTheme: theme
            Layout.fillWidth: true
            Layout.fillHeight: true
            implicitHeight: 240
            padding: 4
            clip: true
            Components.TrackList {
                anchors.fill: parent
                anchors.margins: 4
                visible: libraryModel.count > 0
            }
            ColumnLayout {
                anchors.centerIn: parent
                width: Math.min(parent.width - 32, 360)
                visible: libraryModel.count === 0
                spacing: 12
                Common.AuroraIcon { Layout.alignment: Qt.AlignHCenter; name: searchField.text.length ? "search" : "music"; color: theme.accent; size: 42 }
                Text {
                    Layout.fillWidth: true
                    text: searchField.text.length ? "Ничего не найдено" : "Соберите свою библиотеку"
                    color: theme.text
                    font.pixelSize: 20
                    font.weight: Font.Medium
                    horizontalAlignment: Text.AlignHCenter
                }
                Text {
                    Layout.fillWidth: true
                    text: searchField.text.length ? "Попробуйте другой запрос" : "Добавьте локальные файлы или импортируйте доступную музыку из сервиса"
                    color: theme.subtext
                    font.pixelSize: theme.bodySize
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                }
                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    visible: !searchField.text.length
                    Common.AuroraButton { uiTheme: theme; primary: true; text: "Выбрать файлы"; iconName: "plus"; onClicked: fileDialog.open() }
                    Common.AuroraButton { uiTheme: theme; text: "Папка"; iconName: "folder"; onClicked: folderDialog.open() }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            visible: importManager.busy || importManager.statusText.length || importManager.lastError.length
            Common.AuroraProgress {
                uiTheme: theme
                Layout.fillWidth: true
                visible: importManager.busy
                indeterminate: true
            }
            Text {
                Layout.fillWidth: true
                text: importManager.lastError.length ? importManager.lastError : importManager.statusText
                color: importManager.lastError.length ? theme.danger : theme.subtext
                font.pixelSize: theme.captionSize
                elide: Text.ElideRight
            }
        }
    }

    FileDialog {
        id: fileDialog
        title: "Добавить аудиофайлы"
        fileMode: FileDialog.OpenFiles
        nameFilters: ["Аудио (*.mp3 *.flac *.wav *.ogg *.opus *.m4a *.aac *.wma)", "Все файлы (*)"]
        onAccepted: importManager.importFiles(selectedFiles)
    }
    FolderDialog {
        id: folderDialog
        title: "Сканировать папку"
        onAccepted: importManager.importFolder(selectedFolder)
    }
}
