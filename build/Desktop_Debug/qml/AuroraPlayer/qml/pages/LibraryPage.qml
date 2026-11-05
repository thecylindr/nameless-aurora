// Страница медиатеки: список треков, поиск, фильтры и действия с плейлистами.
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import "../common" as Common
import "../components" as Components

Item {
    id: root
    clip: true
    property bool compact: false
    signal importUrlRequested
    signal createPlaylistRequested
    Theme {
        id: theme
    }
    Components.AmbientBackdrop {
        anchors.fill: parent
        opacity: .48
    }
    Components.AtmosphereOrb {
        width: Math.min(root.width * 1.05, 850)
        height: width
        anchors.right: parent.right
        anchors.rightMargin: -width * .17
        anchors.verticalCenter: parent.verticalCenter
        opacity: .13
        primary: player.atmosphereColor
        interactive: false
    }
    // Определяет заголовок текущей коллекции: избранное, плейлист или вся музыка.
    readonly property string collectionName: {
        if (libraryModel.playlistFilter === -1)
            return "Мне нравится";
        for (let p of libraryModel.playlists)
            if (p.id === libraryModel.playlistFilter)
                return p.name;
        return "Моя музыка";
    }
    function openTrackActions() {
        const row = tracks.itemAtIndex(0);
        return row ? row.openActions() : false;
    }
    function openCollectionActions() {
        collectionMenu.open();
        return collectionMenu.visible;
    }
    function openCollections() {
        collections.popup.open();
        return collections.popup.visible;
    }
    function focusSearch() {
        searchField.forceActiveFocus();
    }
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: compact ? 16 : 32
        spacing: 18
        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 7
                Text {
                    text: "ТВОЯ КОЛЛЕКЦИЯ"
                    color: theme.textMuted
                    font.pixelSize: 10
                    font.letterSpacing: 2
                }
                Text {
                    Layout.fillWidth: true
                    text: root.collectionName
                    color: theme.text
                    font.pixelSize: compact ? 26 : 32
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                }
                Text {
                    text: libraryModel.count + " треков"
                    color: theme.subtext
                    font.pixelSize: 12
                }
            }
            Common.AuroraButton {
                uiTheme: theme
                iconName: "more"
                iconOnly: true
                text: "Действия с коллекцией"
                onClicked: collectionMenu.popup()
            }
        }
        Flow {
            Layout.fillWidth: true
            spacing: 8
            Common.AuroraButton {
                uiTheme: theme
                primary: true
                iconName: "play"
                text: "Слушать"
                enabled: libraryModel.count > 0
                onClicked: player.playIndex(0)
            }
            Common.AuroraButton {
                uiTheme: theme
                iconName: "plus"
                text: "Файлы"
                onClicked: fileDialog.open()
            }
            Common.AuroraButton {
                uiTheme: theme
                iconName: "folder"
                text: "Папка"
                onClicked: folderDialog.open()
            }
            Common.AuroraButton {
                uiTheme: theme
                iconName: "library"
                text: "Альбом"
                onClicked: libraryModel.requestAlbumEdit("")
            }
            Common.AuroraButton {
                uiTheme: theme
                iconName: "download"
                text: "Докачать"
                enabled: libraryModel.count > 0
                onClicked: audioSources.recoverMissing()
            }
            Common.AuroraButton {
                uiTheme: theme
                iconName: "download"
                text: "Импорт"
                onClicked: root.importUrlRequested()
            }
        }
        Common.AuroraField {
            id: searchField
            objectName: "librarySearch"
            uiTheme: theme
            Layout.fillWidth: true
            placeholderText: "Найти трек, исполнителя, альбом"
            iconName: "search"
            text: libraryModel.searchText
            onTextChanged: searchDebounce.restart()
            // Поиск с задержкой 120 мс, чтобы не фильтровать список на каждый символ.
            Timer {
                id: searchDebounce
                interval: 120
                onTriggered: libraryModel.searchText = searchField.text
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Common.AuroraCombo {
                id: collections
                uiTheme: theme
                objectName: "collectionChooser"
                Layout.fillWidth: true
                model: [
                    {
                        name: "Все треки",
                        id: 0
                    },
                    {
                        name: "Мне нравится",
                        id: -1
                    }
                ].concat(libraryModel.playlists)
                textRole: "name"
                currentIndex: {
                    for (let i = 0; i < model.length; i++)
                        if (model[i].id === libraryModel.playlistFilter)
                            return i;
                    return 0;
                }
                onActivated: libraryModel.playlistFilter = model[index].id
            }
            Common.AuroraCombo {
                uiTheme: theme
                Layout.preferredWidth: root.compact ? 130 : 170
                model: ["По исполнителю", "По названию", "Недавно добавлены", "Недавно слушали"]
                currentIndex: ["artist", "title", "recent", "history"].indexOf(libraryModel.sortOrder)
                onActivated: libraryModel.sortOrder = ["artist", "title", "recent", "history"][index]
            }
        }
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Components.TrackList {
                id: tracks
                anchors.fill: parent
                visible: libraryModel.count > 0
            }
            ColumnLayout {
                anchors.centerIn: parent
                width: Math.min(parent.width - 32, 360)
                spacing: 14
                visible: libraryModel.count === 0
                Common.AuroraIcon {
                    Layout.alignment: Qt.AlignHCenter
                    name: "music"
                    color: theme.accent
                    size: 40
                }
                Text {
                    Layout.fillWidth: true
                    text: searchField.text.length ? "Ничего не найдено" : "Здесь будет твоя музыка"
                    color: theme.text
                    font.pixelSize: 22
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                }
                Text {
                    Layout.fillWidth: true
                    text: searchField.text.length ? "Попробуй другое название или исполнителя" : libraryModel.playlistFilter > 0 ? "Добавь треки через меню ⋯ в общей медиатеке" : "Выбери файлы, папку или перенеси плейлист из любимого сервиса"
                    color: theme.subtext
                    font.pixelSize: 13
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    lineHeight: 1.4
                }
            }
        }
        Text {
            Layout.fillWidth: true
            visible: importManager.statusText.length > 0 || importManager.lastError.length > 0
            text: importManager.lastError || importManager.statusText
            color: importManager.lastError ? theme.danger : theme.subtext
            font.pixelSize: 12
            elide: Text.ElideRight
        }
    }
    Common.AuroraMenu {
        id: collectionMenu
        uiTheme: theme
        objectName: "collectionActions"
        Common.AuroraMenuItem {
            uiTheme: theme
            text: "Создать плейлист…"
            onTriggered: root.createPlaylistRequested()
        }
        Common.AuroraMenuItem {
            uiTheme: theme
            text: "Переименовать…"
            enabled: libraryModel.playlistFilter > 0
            onTriggered: {
                renameField.text = root.collectionName;
                renameDialog.open();
            }
        }
        Common.AuroraMenuItem {
            uiTheme: theme
            text: "Экспорт с аудиофайлами…"
            enabled: !libraryExchange.busy
            onTriggered: exportFolder.open()
        }
        Common.AuroraMenuItem {
            uiTheme: theme
            text: "Экспорт списка TXT…"
            enabled: !libraryExchange.busy
            onTriggered: exportText.open()
        }
        MenuSeparator {}
        Common.AuroraMenuItem {
            uiTheme: theme
            text: "Удалить плейлист"
            enabled: libraryModel.playlistFilter > 0
            onTriggered: deleteDialog.open()
        }
    }
    Common.AuroraDialog {
        id: renameDialog
        uiTheme: theme
        parent: Overlay.overlay
        title: "Название плейлиста"
        standardButtons: Dialog.Ok | Dialog.Cancel
        Common.AuroraField {
            id: renameField
            uiTheme: theme
            width: parent.width
            onAccepted: renameDialog.accept()
        }
        onAccepted: if (renameField.text.trim().length)
            libraryModel.renamePlaylist(libraryModel.playlistFilter, renameField.text)
    }
    Common.AuroraDialog {
        id: deleteDialog
        uiTheme: theme
        parent: Overlay.overlay
        title: "Удалить плейлист?"
        standardButtons: Dialog.Yes | Dialog.No
        Text {
            width: parent.width
            text: "Треки останутся в медиатеке."
            color: theme.subtext
            wrapMode: Text.WordWrap
        }
        onAccepted: libraryModel.deletePlaylist(libraryModel.playlistFilter)
    }
    FileDialog {
        id: fileDialog
        title: "Добавить музыку"
        fileMode: FileDialog.OpenFiles
        nameFilters: ["Аудио (*.mp3 *.flac *.wav *.ogg *.opus *.m4a *.aac *.wma *.aiff)"]
        onAccepted: importManager.importFiles(selectedFiles)
    }
    FolderDialog {
        id: folderDialog
        title: "Папка с музыкой"
        onAccepted: importManager.importFolder(selectedFolder)
    }
    FolderDialog {
        id: exportFolder
        title: "Куда сохранить переносимую медиатеку"
        onAccepted: libraryExchange.exportPortable(selectedFolder, libraryModel.playlistFilter)
    }
    FileDialog {
        id: exportText
        title: "Сохранить список треков"
        fileMode: FileDialog.SaveFile
        defaultSuffix: "txt"
        nameFilters: ["Текст (*.txt)"]
        onAccepted: libraryExchange.exportText(selectedFile, libraryModel.playlistFilter)
    }
}
