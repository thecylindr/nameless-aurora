// Диалог сборки и редактирования альбома: выбор названия, обложки и состава треков.
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
    property string previous: ""
    property var tracks: []
    property var selected: ({})
    property url selectedCover: ""
    uiTheme: theme
    title: previous.length ? "Редактировать альбом" : "Собрать альбом"
    standardButtons: Dialog.NoButton
    maxDialogWidth: 680
    Connections {
        target: libraryModel
        // Заполняет форму данными альбома и отмечает треки, которые уже к нему относятся.
        function onAlbumEditRequested(title) {
            root.previous = title;
            name.text = title;
            filter.text = "";
            root.tracks = libraryModel.allTrackDetails();
            root.selectedCover = "";
            let next = {};
            for (const track of root.tracks)
                if (title.length && track.album === title)
                    next[track.trackId] = true;
            root.selected = next;
            root.open();
        }
    }
    ColumnLayout {
        width: parent.width
        spacing: 12
        Common.AuroraField {
            id: name
            uiTheme: theme
            Layout.fillWidth: true
            placeholderText: "Название альбома"
        }
        Common.AuroraButton {
            uiTheme: theme
            text: root.selectedCover.toString().length ? "Обложка выбрана · изменить" : "Добавить обложку альбома"
            iconName: "folder"
            onClicked: cover.open()
        }
        Common.AuroraField {
            id: filter
            uiTheme: theme
            Layout.fillWidth: true
            placeholderText: "Найти трек для альбома"
            iconName: "search"
        }
        Text {
            text: "Выбрано: " + Object.keys(root.selected).length
            color: theme.subtext
            font.pixelSize: 12
        }
        ListView {
            id: list
            Layout.fillWidth: true
            implicitHeight: Math.min(310, contentHeight)
            clip: true
            spacing: 4
            // Фильтрация списка по подстроке в названии или исполнителе без учёта регистра.
            model: root.tracks.filter(track => !filter.text.length || (track.title + " " + track.artist).toLowerCase().indexOf(filter.text.toLowerCase()) >= 0)
            ScrollBar.vertical: Common.AuroraScrollBar {
                uiTheme: theme
            }
            delegate: CheckDelegate {
                required property var modelData
                width: list.width
                height: 52
                text: modelData.title + " · " + modelData.artist
                checked: !!root.selected[modelData.trackId]
                background: Rectangle {
                    radius: 10
                    color: parent.hovered ? theme.hover : theme.surfaceHigh
                }
                contentItem: Text {
                    text: parent.text
                    color: theme.text
                    font.pixelSize: 12
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                    rightPadding: 38
                }
                onClicked: {
                    // Копируем словарь выбора, чтобы смена отметки перезапустила привязки.
                    let next = Object.assign({}, root.selected);
                    if (next[modelData.trackId])
                        delete next[modelData.trackId];
                    else
                        next[modelData.trackId] = true;
                    root.selected = next;
                }
            }
        }
        Text {
            Layout.fillWidth: true
            text: "Отметь состав альбома. Снятые из этого альбома треки останутся в медиатеке."
            color: theme.textMuted
            font.pixelSize: 11
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
                text: "Сохранить альбом"
                primary: true
                enabled: name.text.trim().length > 0 && Object.keys(root.selected).length > 0
                onClicked: {
                    if (libraryModel.setAlbumTracks(root.previous, name.text, Object.keys(root.selected).map(Number), root.selectedCover))
                        root.close();
                }
            }
        }
    }
    FileDialog {
        id: cover
        title: "Обложка альбома"
        nameFilters: ["Изображения (*.jpg *.jpeg *.png *.webp)"]
        onAccepted: root.selectedCover = selectedFile
    }
}
