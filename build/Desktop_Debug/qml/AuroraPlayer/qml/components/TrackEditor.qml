// Диалог редактирования трека: название, исполнитель, альбом и обложка.
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import "../common" as Common

Common.AuroraDialog {
    id: root
    Theme {
        id: theme
    }
    property var track: ({})
    property url selectedCover: ""
    uiTheme: theme
    title: "Редактировать трек"
    standardButtons: Dialog.NoButton
    Connections {
        target: libraryModel
        function onEditRequested(id) {
            root.track = libraryModel.trackDetails(id);
            name.text = root.track.title || "";
            artist.text = root.track.artist || "";
            album.text = root.track.album || "";
            root.selectedCover = "";
            root.open();
        }
    }
    ColumnLayout {
        width: parent.width
        spacing: 14
        RowLayout {
            AlbumArt {
                Layout.preferredWidth: 88
                Layout.preferredHeight: 88
                // Выбранный файл декодируется из URL в обычный путь для предпросмотра.
                path: root.selectedCover.toString().length ? decodeURIComponent(root.selectedCover.toString().replace("file://", "")) : root.track.coverPath || ""
                seed: root.track.title || "Aurora"
            }
            Common.AuroraButton {
                uiTheme: theme
                text: "Выбрать обложку"
                iconName: "folder"
                onClicked: cover.open()
            }
        }
        Common.AuroraField {
            id: name
            uiTheme: theme
            Layout.fillWidth: true
            placeholderText: "Название трека"
        }
        Common.AuroraField {
            id: artist
            uiTheme: theme
            Layout.fillWidth: true
            placeholderText: "Исполнитель"
        }
        Common.AuroraField {
            id: album
            uiTheme: theme
            Layout.fillWidth: true
            placeholderText: "Альбом"
        }
        Text {
            Layout.fillWidth: true
            text: "Изменения сохраняются в медиатеке. Исходный файл и история прослушивания сохраняются."
            color: theme.subtext
            font.pixelSize: 12
            wrapMode: Text.WordWrap
        }
        RowLayout {
            Layout.alignment: Qt.AlignRight
            Common.AuroraButton {
                uiTheme: theme
                text: "Отмена"
                onClicked: root.close()
            }
            Common.AuroraButton {
                uiTheme: theme
                text: "Сохранить"
                primary: true
                enabled: name.text.trim().length > 0
                // Сначала сохраняем обложку (если выбрана), затем метаданные; при любой ошибке диалог остаётся открытым.
                onClicked: {
                    if (!root.selectedCover.toString().length || libraryModel.setTrackCover(root.track.trackId, root.selectedCover))
                        if (libraryModel.updateMetadata(root.track.trackId, name.text, artist.text, album.text))
                            root.close();
                }
            }
        }
    }
    FileDialog {
        id: cover
        title: "Обложка трека"
        nameFilters: ["Изображения (*.jpg *.jpeg *.png *.webp)"]
        onAccepted: root.selectedCover = selectedFile
    }
}
