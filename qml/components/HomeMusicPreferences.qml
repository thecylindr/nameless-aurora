import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ".."
import "../common" as Common

Common.AuroraDialog {
    id: root
    signal discoveryRequested
    Theme {
        id: theme
    }
    uiTheme: theme
    title: "Настроить музыку"
    standardButtons: Dialog.NoButton

    function setPreference(key, value) {
        const next = Object.assign({}, homeFeed.preferences);
        next[key] = value;
        homeFeed.preferences = next;
    }
    function togglePlaylist(id) {
        const excluded = homeFeed.preferences.excludedPlaylists.slice();
        const index = excluded.indexOf(id);
        if (index >= 0)
            excluded.splice(index, 1);
        else
            excluded.push(id);
        setPreference("excludedPlaylists", excluded);
    }

    ColumnLayout {
        width: parent.width
        spacing: 14
        Text {
            Layout.fillWidth: true
            text: "Что чаще предлагать из коллекции?"
            color: theme.text
            font.pixelSize: 15
            font.weight: Font.Medium
        }
        Common.AuroraSegmented {
            Layout.fillWidth: true
            uiTheme: theme
            model: ["Баланс", "Новое", "Любимое"]
            currentIndex: ["balanced", "discovery", "familiar"].indexOf(homeFeed.preferences.discoveryMode)
            onOptionSelected: function (index, value) {
                root.setPreference("discoveryMode", ["balanced", "discovery", "familiar"][index]);
            }
        }
        Text {
            text: "Источник «Моей волны»"
            color: theme.subtext
            font.pixelSize: 12
        }
        Common.AuroraCombo {
            Layout.fillWidth: true
            uiTheme: theme
            model: ["Автоматически", "Моя коллекция", "Яндекс Музыка", "VK Музыка"]
            currentIndex: ["auto", "local", "yandex", "vk"].indexOf(homeFeed.preferences.waveSource)
            onActivated: function (index) {
                root.setPreference("waveSource", ["auto", "local", "yandex", "vk"][index]);
            }
        }
        Common.AuroraSwitch {
            Layout.fillWidth: true
            uiTheme: theme
            label: "Только файлы на устройстве"
            subtext: "Для подборок из коллекции. Рекомендации сервисов доступны отдельно."
            checked: homeFeed.preferences.onlyDownloaded
            onToggled: root.setPreference("onlyDownloaded", checked)
        }
        Common.AuroraSwitch {
            Layout.fillWidth: true
            uiTheme: theme
            label: "Яндекс Музыка на главной"
            checked: homeFeed.preferences.showYandex
            onToggled: root.setPreference("showYandex", checked)
        }
        Common.AuroraSwitch {
            Layout.fillWidth: true
            uiTheme: theme
            label: "VK Музыка на главной"
            checked: homeFeed.preferences.showVk
            onToggled: root.setPreference("showVk", checked)
        }
        Text {
            text: "Из каких плейлистов собирать"
            color: theme.text
            font.pixelSize: 15
            font.weight: Font.Medium
        }
        Common.AuroraSwitch {
            Layout.fillWidth: true
            uiTheme: theme
            label: "Треки вне плейлистов"
            checked: homeFeed.preferences.includeUnsorted
            onToggled: root.setPreference("includeUnsorted", checked)
        }
        ListView {
            id: playlists
            Layout.fillWidth: true
            Layout.preferredHeight: Math.min(180, contentHeight)
            clip: true
            spacing: 4
            model: libraryModel.playlists
            boundsBehavior: Flickable.StopAtBounds
            delegate: Common.AuroraSwitch {
                required property var modelData
                width: playlists.width
                uiTheme: theme
                label: modelData.name
                checked: homeFeed.preferences.excludedPlaylists.indexOf(modelData.id) < 0
                onToggled: root.togglePlaylist(modelData.id)
            }
        }
        Text {
            text: "Что искать среди новинок?"
            color: theme.text
            font.pixelSize: 15
            font.weight: Font.Medium
        }
        Common.AuroraField {
            id: taste
            Layout.fillWidth: true
            uiTheme: theme
            text: musicSearch.taste
            placeholderText: "Жанры или исполнители: фонк, джаз, инди…"
            onEditingFinished: musicSearch.taste = text
        }
        Text {
            Layout.fillWidth: true
            text: "Этот запрос используется для поиска новинок. Персональные рекомендации VK и Яндекса приходят из твоих аккаунтов."
            color: theme.subtext
            font.pixelSize: 12
            wrapMode: Text.WordWrap
        }
        Common.AuroraButton {
            uiTheme: theme
            text: "Подобрать новинки"
            iconName: "search"
            onClicked: {
                musicSearch.taste = taste.text;
                root.close();
                root.discoveryRequested();
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Text {
                Layout.fillWidth: true
                text: "Настройки сохраняются автоматически"
                color: theme.textMuted
                font.pixelSize: 10
                wrapMode: Text.WordWrap
            }
            Common.AuroraButton {
                uiTheme: theme
                primary: true
                text: "Готово"
                onClicked: root.close()
            }
        }
    }
}
