// Главная: подборки по плейлистам и персональная волна Яндекс Музыки.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ".."
import "../common" as Common
import "../components" as Components

Item {
    id: root

    property bool compact: false
    property int period: 0
    property var selectedMix: ({
    })
    readonly property var section: homeFeed.sections[period] || ({
    })
    readonly property var mixes: section.mixes || []
    readonly property string waveSource: homeFeed.preferences.waveSource !== "auto" ? homeFeed.preferences.waveSource : homeFeed.yandexConnected && homeFeed.preferences.showYandex ? "yandex" : homeFeed.vkConnected && homeFeed.preferences.showVk ? "vk" : "local"

    signal importRequested()
    signal libraryRequested()
    signal playlistsRequested()
    signal filesRequested()
    signal searchRequested()
    signal discoveryRequested()

    function openMix(mix) {
        selectedMix = mix;
        mixPage = 0;
        mixDialog.open();
    }

    function showProviders() {
        const flick = scroll.contentItem as Flickable;
        if (flick)
            flick.contentY = Math.max(0, Math.min(yandexShelf.y, flick.contentHeight - flick.height));

    }

    function openPreferences() {
        preferencesDialog.open();
    }

    Component.onCompleted: {
        if (homeFeed.yandexConnected && homeFeed.preferences.showYandex)
            homeFeed.refreshYandex();

        if (homeFeed.vkConnected && homeFeed.preferences.showVk)
            homeFeed.refreshVk();

    }

    Theme {
        id: theme
    }

    ScrollView {
        id: scroll

        anchors.fill: parent
        clip: true
        contentWidth: availableWidth
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        ColumnLayout {
            width: scroll.availableWidth - (root.compact ? 32 : 64)
            x: root.compact ? 16 : 32
            spacing: 22

            Item {
                implicitHeight: 6
            }

            RowLayout {
                Layout.fillWidth: true

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 6

                    Text {
                        text: new Date(root.section.date ? root.section.date + "T12:00:00" : Date.now()).toLocaleDateString(Qt.locale("ru_RU"), "dddd, d MMMM").toUpperCase()
                        color: theme.subtext
                        font.pixelSize: 10
                        font.letterSpacing: 1.6
                    }

                    Text {
                        text: "Твоя музыка сегодня"
                        color: theme.text
                        font.pixelSize: root.compact ? 25 : 32
                        font.weight: Font.DemiBold
                        font.letterSpacing: -0.6
                    }

                }

                Common.AuroraButton {
                    uiTheme: theme
                    iconName: "plus"
                    text: root.compact ? "" : "Добавить музыку"
                    iconOnly: root.compact
                    onClicked: root.filesRequested()
                }

            }

            Rectangle {
                id: hero

                visible: settingsStore.homeHeroVisible
                Layout.fillWidth: true
                implicitHeight: root.compact ? 250 : 236
                radius: 24
                clip: true
                color: theme.surfaceHigh

                Components.AmbientBackdrop {
                    anchors.fill: parent
                    animated: true
                }

                Components.AtmosphereOrb {
                    width: Math.min(parent.width * 0.65, 360)
                    height: width
                    anchors.right: parent.right
                    anchors.rightMargin: root.compact ? -92 : 4
                    anchors.verticalCenter: parent.verticalCenter
                    opacity: root.compact ? 0.42 : 0.85
                }

                Components.CornerCover {
                    anchors.fill: parent
                    radius: 24
                    color: theme.base
                    z: 5
                }

                ColumnLayout {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.margins: root.compact ? 22 : 24
                    width: parent.width * (root.compact ? 0.88 : 0.68)
                    spacing: 8

                    Text {
                        text: root.waveSource === "vk" ? "ТВОЯ ВОЛНА · VK МУЗЫКА" : root.waveSource === "yandex" ? "ТВОЯ ВОЛНА · ЯНДЕКС" : "ИЗ ТВОЕЙ КОЛЛЕКЦИИ"
                        color: theme.accent
                        font.pixelSize: 10
                        font.letterSpacing: 1.5
                    }

                    Text {
                        text: "Моя волна"
                        color: theme.text
                        font.pixelSize: root.compact ? 37 : 44
                        font.weight: Font.DemiBold
                        font.letterSpacing: -1.5
                    }

                    Text {
                        Layout.fillWidth: true
                        text: root.waveSource === "yandex" ? "Рекомендации твоего аккаунта Яндекса.\nНовые треки и любимое — в одном потоке." : root.waveSource === "vk" ? "Персональная подборка из твоей VK Музыки.\nНовые треки и любимые исполнители рядом." : "Открывай непрослушанное в своих плейлистах.\nНастрой настроение и источники музыки."
                        color: theme.subtext
                        font.pixelSize: root.compact ? 12 : 14
                        lineHeight: 1.35
                        wrapMode: Text.WordWrap
                    }

                    Item {
                        Layout.fillHeight: true
                    }

                    RowLayout {
                        spacing: 10

                        Common.AuroraButton {
                            uiTheme: theme
                            primary: true
                            iconName: "play"
                            text: homeFeed.busy || homeFeed.vkBusy ? "Собираем музыку…" : "Включить волну"
                            enabled: !homeFeed.busy && !homeFeed.vkBusy
                            onClicked: {
                                if (homeFeed.yandexConnected || homeFeed.vkConnected || homeFeed.sections.length)
                                    homeFeed.playWave();
                                else
                                    root.filesRequested();
                            }
                        }

                        Common.AuroraButton {
                            uiTheme: theme
                            iconOnly: root.compact
                            iconName: "search"
                            text: "Найти новое"
                            onClicked: root.discoveryRequested()
                        }

                    }

                }

            }

            Text {
                Layout.fillWidth: true
                visible: homeFeed.playbackStatus.length > 0 && !mixDialog.visible
                text: homeFeed.playbackStatus
                color: theme.accent
                font.pixelSize: 12
                wrapMode: Text.WordWrap
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 12

                RowLayout {
                    Layout.fillWidth: true

                    Text {
                        Layout.fillWidth: true
                        text: "Собрано для тебя"
                        color: theme.text
                        font.pixelSize: 22
                        font.weight: Font.DemiBold
                    }

                    Common.AuroraButton {
                        uiTheme: theme
                        iconName: "settings"
                        iconOnly: root.compact
                        text: "Настроить"
                        implicitHeight: 34
                        onClicked: root.openPreferences()
                    }

                    Common.AuroraButton {
                        uiTheme: theme
                        iconName: "library"
                        iconOnly: root.compact
                        text: "Плейлисты"
                        implicitHeight: 34
                        onClicked: root.playlistsRequested()
                    }

                }

                Common.AuroraSegmented {
                    Layout.fillWidth: true
                    Layout.maximumWidth: 470
                    uiTheme: theme
                    model: ["На сегодня", "На 3 дня", root.compact ? "Эта неделя" : "На этой неделе"]
                    currentIndex: root.period
                    onOptionSelected: function(index, value) {
                        root.period = index;
                    }
                }

                Text {
                    Layout.fillWidth: true
                    text: root.section.subtitle || (libraryModel.totalCount ? "Нет треков с учётом настроек подбора" : "Добавь плейлисты — здесь появятся твои подборки")
                    color: theme.subtext
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                }

            }

            ListView {
                id: mixShelf

                Layout.fillWidth: true
                implicitHeight: 336
                visible: root.mixes.length > 0
                orientation: ListView.Horizontal
                model: root.mixes
                spacing: 14
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                onModelChanged: positionViewAtBeginning()

                delegate: Components.HomeMixCard {
                    required property var modelData
                    required property int index

                    mix: modelData
                    tint: ["#493e75", "#275d60", "#74483d", "#3b526e"][index % 4]
                    onPlayRequested: homeFeed.playMix(modelData)
                    onOpened: root.openMix(modelData)
                }

            }

            Rectangle {
                Layout.fillWidth: true
                visible: !root.mixes.length
                implicitHeight: emptyContent.implicitHeight + 40
                color: theme.surface
                radius: 16
                border.color: theme.line

                ColumnLayout {
                    id: emptyContent

                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 20
                    spacing: 14

                    Text {
                        Layout.fillWidth: true
                        text: libraryModel.totalCount ? "Проверь выбранные плейлисты и фильтр файлов в настройках музыки." : "Перенеси плейлисты из VK или Яндекса либо добавь файлы. Подборки появятся даже у треков, которые ты ещё не слушал."
                        color: theme.subtext
                        font.pixelSize: 14
                        wrapMode: Text.WordWrap
                    }

                    Common.AuroraButton {
                        uiTheme: theme
                        text: libraryModel.totalCount ? "Настроить подборки" : "Перенести плейлисты"
                        iconName: "settings"
                        onClicked: libraryModel.totalCount ? root.openPreferences() : root.importRequested()
                    }

                }

            }

            Components.HomeProviderShelf {
                id: yandexShelf

                Layout.fillWidth: true
                visible: homeFeed.preferences.showYandex
                title: "Из твоей Яндекс Музыки"
                compact: root.compact
                connected: homeFeed.yandexConnected
                busy: homeFeed.busy
                status: homeFeed.status
                tracks: homeFeed.yandexTracks
                onConnectRequested: webLogin.openNative("Яндекс Музыка")
                onOpened: {
                    providerPlaylist.provider = "yandex";
                    providerPlaylist.open();
                }
                onRefreshRequested: homeFeed.refreshYandex(true)
                onPlayRequested: function(index) {
                    homeFeed.playYandex(index);
                }
            }

            Components.HomeProviderShelf {
                id: vkShelf

                Layout.fillWidth: true
                visible: homeFeed.preferences.showVk
                title: "Из твоей VK Музыки"
                compact: root.compact
                connected: homeFeed.vkConnected
                busy: homeFeed.vkBusy
                status: homeFeed.vkStatus
                tracks: homeFeed.vkTracks
                onConnectRequested: webLogin.openNative("VK")
                onOpened: {
                    providerPlaylist.provider = "vk";
                    providerPlaylist.open();
                }
                onRefreshRequested: homeFeed.refreshVk(true)
                onPlayRequested: function(index) {
                    homeFeed.playVk(index);
                }
            }

            RowLayout {
                Layout.fillWidth: true

                Common.AuroraButton {
                    Layout.fillWidth: true
                    uiTheme: theme
                    text: "Мне нравится"
                    iconName: "heart"
                    onClicked: {
                        libraryModel.playlistFilter = -1;
                        root.libraryRequested();
                    }
                }

                Common.AuroraButton {
                    Layout.fillWidth: true
                    uiTheme: theme
                    text: "Вся музыка"
                    iconName: "library"
                    onClicked: {
                        libraryModel.playlistFilter = 0;
                        root.libraryRequested();
                    }
                }

            }

            Item {
                implicitHeight: 16
            }

        }

    }

    Components.HomeMusicPreferences {
        id: preferencesDialog

        parent: Overlay.overlay
        onDiscoveryRequested: root.discoveryRequested()
    }

    Components.ProviderPlaylistDialog {
        id: providerPlaylist

        parent: Overlay.overlay
    }

    Common.AuroraDialog {
        id: mixDialog

        parent: Overlay.overlay
        uiTheme: theme
        title: root.selectedMix.title || "Подборка"
        standardButtons: Dialog.NoButton

        ColumnLayout {
            width: parent.width
            spacing: 12

            Text {
                Layout.fillWidth: true
                text: (root.selectedMix.description || root.selectedMix.subtitle || "") + " · " + (root.selectedMix.count || 0) + " треков"
                color: theme.subtext
                wrapMode: Text.WordWrap
            }

            Common.AuroraButton {
                uiTheme: theme
                primary: true
                text: "Слушать подборку"
                iconName: "play"
                onClicked: {
                    homeFeed.playMix(root.selectedMix);
                }
            }

            Text {
                Layout.fillWidth: true
                visible: homeFeed.playbackStatus.length > 0
                text: homeFeed.playbackStatus
                color: theme.accent
                font.pixelSize: 12
                wrapMode: Text.WordWrap
            }

            ListView {
                id: mixTracks

                Layout.fillWidth: true
                Layout.preferredHeight: Math.max(120, Math.min(340, root.height - 280))
                clip: true
                model: (root.selectedMix.tracks || []).slice(root.mixPage * 12, (root.mixPage + 1) * 12)
                boundsBehavior: Flickable.StopAtBounds

                delegate: ItemDelegate {
                    id: trackRow

                    required property var modelData

                    width: mixTracks.width
                    implicitHeight: 64
                    onClicked: {
                        homeFeed.playMix(root.selectedMix, modelData.trackId);
                    }

                    background: Rectangle {
                        radius: 8
                        color: trackRow.hovered ? theme.hover : "transparent"
                    }

                    contentItem: RowLayout {
                        spacing: 12

                        Components.AlbumArt {
                            Layout.preferredWidth: 40
                            Layout.preferredHeight: 40
                            path: modelData.coverPath || ""
                            seed: modelData.title
                            cornerRadius: 7
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            Text {
                                Layout.fillWidth: true
                                text: modelData.title
                                color: theme.text
                                font.pixelSize: 13
                                elide: Text.ElideRight
                            }

                            Text {
                                Layout.fillWidth: true
                                text: modelData.artist
                                color: theme.subtext
                                font.pixelSize: 11
                                elide: Text.ElideRight
                            }

                        }

                        Common.AuroraButton {
                            readonly property bool pending: homeFeed.resolvingId === trackRow.modelData.trackId
                            readonly property bool current: player.currentTrackId === trackRow.modelData.trackId || (trackRow.modelData.url && player.streamKey === trackRow.modelData.url)

                            uiTheme: theme
                            iconOnly: true
                            primary: true
                            iconName: pending ? "close" : current && player.playing ? "pause" : "play"
                            text: pending ? "Отменить подключение" : current && player.playing ? "Пауза" : "Слушать трек"
                            enabled: trackRow.modelData.canListen
                            implicitWidth: 40
                            implicitHeight: 40
                            onClicked: {
                                if (pending)
                                    player.stop();
                                else
                                    homeFeed.playTrack(trackRow.modelData.trackId);
                            }
                        }

                        Common.AuroraButton {
                            uiTheme: theme
                            iconOnly: true
                            iconName: "search"
                            text: "Выбрать другой источник"
                            implicitWidth: 36
                            implicitHeight: 40
                            visible: !trackRow.modelData.downloaded
                            onClicked: {
                                mixDialog.close();
                                homeFeed.findSource(trackRow.modelData.trackId);
                            }
                        }

                    }

                }

            }

            RowLayout {
                Layout.fillWidth: true

                Text {
                    Layout.fillWidth: true
                    text: "Страница " + (root.mixPage + 1) + " из " + root.mixPageCount
                    color: theme.subtext
                    font.pixelSize: 11
                }

                Common.AuroraButton {
                    uiTheme: theme
                    iconOnly: true
                    iconName: "previous"
                    text: "Предыдущая страница"
                    implicitHeight: 34
                    enabled: root.mixPage > 0
                    onClicked: {
                        root.mixPage--;
                        mixTracks.positionViewAtBeginning();
                    }
                }

                Common.AuroraButton {
                    uiTheme: theme
                    iconOnly: true
                    iconName: "next"
                    text: "Следующая страница"
                    implicitHeight: 34
                    enabled: root.mixPage + 1 < root.mixPageCount
                    onClicked: {
                        root.mixPage++;
                        mixTracks.positionViewAtBeginning();
                    }
                }

            }

            Common.AuroraButton {
                Layout.alignment: Qt.AlignRight
                uiTheme: theme
                text: "Закрыть"
                onClicked: mixDialog.close()
            }

        }

    }

}
