// Страница поиска музыки: онлайн-поиск по источникам, прослушивание и загрузка результатов.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../common" as Common
import "../components" as Components

Item {
    id: root
    property bool compact: false
    property bool preferencesOpen: false
    property string resultSource: "Все"
    property int pageIndex: 0
    property int pageSize: 10
    property string displayedQuery: ""
    // Фильтрация по источнику и разбивка результатов на страницы.
    readonly property var filteredResults: musicSearch.results.filter(item => resultSource === "Все" || item.source === resultSource)
    readonly property int pageCount: Math.max(1, Math.ceil(filteredResults.length / pageSize))
    readonly property var pageResults: filteredResults.slice(pageIndex * pageSize, (pageIndex + 1) * pageSize)
    onResultSourceChanged: pageIndex = 0
    onPageSizeChanged: pageIndex = 0
    onPageIndexChanged: list.positionViewAtBeginning()
    onPageCountChanged: pageIndex = Math.min(pageIndex, pageCount - 1)
    Connections {
        target: musicSearch
        function onChanged() {
            if (root.displayedQuery !== musicSearch.query) {
                root.displayedQuery = musicSearch.query;
                root.pageIndex = 0;
            }
        }
    }
    signal libraryRequested
    Theme {
        id: theme
    }
    function focusSearch() {
        query.forceActiveFocus();
        query.selectAll();
    }
    function searchNow() {
        delayedSearch.stop();
        if (query.text.trim())
            musicSearch.search(query.text);
    }
    function toggleSource(source) {
        let selected = musicSearch.enabledSources.slice();
        const i = selected.indexOf(source);
        if (i < 0)
            selected.push(source);
        else
            selected.splice(i, 1);
        musicSearch.enabledSources = selected;
        if (query.text.trim())
            delayedSearch.restart();
    }
    // Поиск с задержкой 550 мс после ввода, чтобы не отправлять запрос на каждый символ.
    Timer {
        id: delayedSearch
        interval: 550
        onTriggered: root.searchNow()
    }
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.compact ? 16 : 28
        spacing: 14
        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 5
                Text {
                    text: "ЗА ПРЕДЕЛАМИ КОЛЛЕКЦИИ"
                    color: theme.accent
                    font.pixelSize: 9
                    font.letterSpacing: 2
                }
                Text {
                    text: "Найди свой звук"
                    color: theme.text
                    font.pixelSize: root.compact ? 25 : 30
                    font.weight: Font.DemiBold
                }
            }
            Common.AuroraButton {
                uiTheme: theme
                text: root.compact ? "" : "Твой вкус"
                iconName: "settings"
                iconOnly: root.compact
                outlined: root.preferencesOpen
                onClicked: root.preferencesOpen = !root.preferencesOpen
            }
        }
        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            Common.AuroraField {
                id: query
                uiTheme: theme
                Layout.fillWidth: true
                iconName: "search"
                placeholderText: "Трек, исполнитель или настроение"
                onAccepted: root.searchNow()
                onTextEdited: if (text.trim().length >= 3)
                    delayedSearch.restart()
                else
                    delayedSearch.stop()
            }
            Common.AuroraButton {
                uiTheme: theme
                text: musicSearch.busy ? "Стоп" : "Найти"
                primary: !musicSearch.busy
                enabled: musicSearch.busy || query.text.trim().length > 0
                onClicked: musicSearch.busy ? musicSearch.cancel() : root.searchNow()
            }
        }
        Flow {
            Layout.fillWidth: true
            spacing: 8
            Repeater {
                model: ["YouTube", "Zaycev.net", "VK"]
                delegate: Common.AuroraButton {
                    required property string modelData
                    uiTheme: theme
                    text: modelData
                    iconName: musicSearch.enabledSources.indexOf(modelData) >= 0 ? "check" : "plus"
                    outlined: musicSearch.enabledSources.indexOf(modelData) >= 0
                    implicitHeight: 36
                    onClicked: root.toggleSource(modelData)
                }
            }
        }
        Rectangle {
            visible: root.preferencesOpen
            Layout.fillWidth: true
            implicitHeight: preferences.implicitHeight + 28
            radius: 16
            color: theme.surface
            border.color: theme.line
            ColumnLayout {
                id: preferences
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 14
                spacing: 10
                Text {
                    Layout.fillWidth: true
                    text: "Что тебе нравится?"
                    color: theme.text
                    font.pixelSize: 14
                    font.weight: Font.Medium
                }
                Common.AuroraField {
                    uiTheme: theme
                    Layout.fillWidth: true
                    text: musicSearch.taste
                    placeholderText: "Например: фонк, джаз, инди, synthwave"
                    onEditingFinished: musicSearch.taste = text
                }
                Text {
                    Layout.fillWidth: true
                    text: "YouTube и Zaycev подберут записи по твоему вкусу. VK — музыку из своего раздела рекомендаций, если он доступен аккаунту. Выдача обновляется по запросу."
                    color: theme.subtext
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                }
                Common.AuroraButton {
                    uiTheme: theme
                    text: "Подобрать музыку"
                    iconName: "shuffle"
                    primary: true
                    enabled: !musicSearch.busy
                    onClicked: {
                        delayedSearch.stop();
                        musicSearch.discover();
                    }
                }
            }
        }
        Flow {
            Layout.fillWidth: true
            spacing: 12
            Repeater {
                model: musicSearch.enabledSources
                delegate: Text {
                    required property string modelData
                    text: modelData + " · " + (musicSearch.sourceStates[modelData] || "готов к поиску")
                    color: theme.textMuted
                    font.pixelSize: 11
                    width: Math.min(implicitWidth, root.width - 64)
                    wrapMode: Text.WordWrap
                }
            }
        }
        Text {
            Layout.fillWidth: true
            text: player.streamLoading ? player.streamStatus : musicSearch.status || "Слушай онлайн, а понравившееся скачивай отдельно."
            color: theme.subtext
            font.pixelSize: 12
            wrapMode: Text.WordWrap
        }
        Common.AuroraProgress {
            uiTheme: theme
            Layout.fillWidth: true
            visible: musicSearch.busy || musicSearch.resolving || musicSearch.auditionKey.length > 0
            indeterminate: true
        }
        RowLayout {
            visible: musicSearch.results.length > 0
            Layout.fillWidth: true
            Text {
                text: "Записи"
                color: theme.text
                font.pixelSize: 17
                font.weight: Font.DemiBold
                Layout.fillWidth: true
            }
            Common.AuroraCombo {
                uiTheme: theme
                Layout.preferredWidth: root.compact ? 140 : 180
                model: ["Все", "YouTube", "Zaycev.net", "VK"]
                onActivated: root.resultSource = currentText
            }
        }
        ListView {
            id: list
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 8
            model: root.pageResults
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: Common.AuroraScrollBar {
                uiTheme: theme
            }
            delegate: Rectangle {
                id: row
                required property var modelData
                readonly property bool connecting: musicSearch.auditionKey === modelData.key || (player.streamKey === modelData.key && player.streamLoading)
                // Ссылки на результаты и статус нужны, чтобы пересчитать привязку после обновления выдачи.
                readonly property int savedId: {
                    const revision = musicSearch.results;
                    const state = musicSearch.status;
                    return musicSearch.libraryId(modelData.key);
                }
                readonly property bool downloaded: {
                    const revision = libraryModel.downloadedCount;
                    return savedId > 0 && libraryModel.trackDetails(savedId).downloaded === true;
                }
                readonly property bool current: player.streamKey === modelData.key || (savedId > 0 && player.currentTrackId === savedId)
                width: list.width - 10
                height: root.compact ? 94 : 90
                radius: 16
                color: theme.surface
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: root.compact ? 10 : 14
                    Components.AlbumArt {
                        Layout.preferredWidth: root.compact ? 48 : 56
                        Layout.preferredHeight: width
                        path: row.modelData.coverUrl || ""
                        seed: row.modelData.title
                        cornerRadius: 10
                    }
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 5
                        Text {
                            Layout.fillWidth: true
                            text: row.modelData.title
                            color: theme.text
                            font.pixelSize: 13
                            font.weight: Font.Medium
                            elide: Text.ElideRight
                            maximumLineCount: 1
                        }
                        Text {
                            Layout.fillWidth: true
                            text: row.modelData.artist || "Неизвестный исполнитель"
                            color: theme.subtext
                            font.pixelSize: 11
                            elide: Text.ElideRight
                        }
                        RowLayout {
                            spacing: 8
                            Rectangle {
                                implicitWidth: platformLabel.implicitWidth + 16
                                implicitHeight: 21
                                radius: 7
                                color: Qt.rgba(platformLabel.color.r, platformLabel.color.g, platformLabel.color.b, 0.13)
                                Text {
                                    id: platformLabel
                                    anchors.centerIn: parent
                                    text: row.modelData.source
                                    color: row.modelData.source === "YouTube" ? "#ff8c98" : row.modelData.source === "VK" ? "#8abbff" : "#f2c879"
                                    font.pixelSize: 10
                                    font.weight: Font.DemiBold
                                }
                            }
                            Text {
                                text: row.modelData.durationMs > 0 ? Math.floor(row.modelData.durationMs / 60000) + ":" + String(Math.floor(row.modelData.durationMs / 1000) % 60).padStart(2, "0") : ""
                                color: theme.textMuted
                                font.pixelSize: 10
                            }
                            Text {
                                visible: row.downloaded
                                text: "На устройстве"
                                color: theme.good
                                font.pixelSize: 10
                            }
                        }
                    }
                    Common.AuroraButton {
                        uiTheme: theme
                        text: "Открыть в сервисе"
                        iconName: "link"
                        iconOnly: true
                        implicitWidth: 34
                        implicitHeight: 34
                        visible: !root.compact
                        onClicked: musicSearch.openInService(row.modelData.key)
                    }
                    Common.AuroraButton {
                        uiTheme: theme
                        iconOnly: true
                        implicitWidth: 40
                        implicitHeight: 40
                        iconName: row.connecting ? "" : row.current && player.playing ? "pause" : "play"
                        Common.ProgressRing {
                            anchors.centerIn: parent
                            width: 23
                            height: 23
                            visible: row.connecting
                            indeterminate: true
                            showLabel: false
                            strokeWidth: 2
                            color: parent.foreground
                        }
                        text: row.connecting ? "Отменить подключение" : row.current && player.playing ? "Пауза" : "Слушать онлайн"
                        primary: row.current
                        onClicked: {
                            if (row.connecting) {
                                musicSearch.cancelAudition();
                                if (player.streamKey === row.modelData.key)
                                    player.stop();
                            } else if (row.current && (player.playing || !player.lastError.length))
                                player.toggle();
                            else
                                musicSearch.listen(row.modelData.key);
                        }
                    }
                    Common.AuroraButton {
                        uiTheme: theme
                        text: row.downloaded ? "Скачано" : "Скачать"
                        iconName: row.downloaded ? "check" : "download"
                        iconOnly: root.compact
                        outlined: !row.downloaded
                        enabled: !musicSearch.resolving
                        onClicked: row.downloaded ? root.libraryRequested() : musicSearch.download(row.modelData.key)
                    }
                }
            }
            Column {
                visible: !musicSearch.results.length && !musicSearch.busy
                anchors.centerIn: parent
                width: Math.min(parent.width - 30, 400)
                spacing: 16
                Common.AuroraIcon {
                    anchors.horizontalCenter: parent.horizontalCenter
                    name: "search"
                    size: 38
                    color: theme.accent
                }
                Text {
                    width: parent.width
                    text: "Музыка начинается с любопытства"
                    color: theme.text
                    font.pixelSize: 19
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                }
                Text {
                    width: parent.width
                    text: "Ищи название или настроение. Слушай запись кнопкой ▶ без сохранения. «Скачать» добавит аудиофайл в библиотеку."
                    color: theme.subtext
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    lineHeight: 1.4
                }
                Common.AuroraButton {
                    anchors.horizontalCenter: parent.horizontalCenter
                    uiTheme: theme
                    text: "Подобрать по вкусу"
                    iconName: "shuffle"
                    onClicked: musicSearch.discover()
                }
            }
        }
        RowLayout {
            visible: root.filteredResults.length > 0
            Layout.fillWidth: true
            spacing: 8
            Text {
                Layout.fillWidth: true
                text: (root.pageIndex * root.pageSize + 1) + "–" + Math.min((root.pageIndex + 1) * root.pageSize, root.filteredResults.length) + " из " + root.filteredResults.length
                color: theme.subtext
                font.pixelSize: 11
            }
            Common.AuroraCombo {
                visible: !root.compact
                uiTheme: theme
                Layout.preferredWidth: 138
                model: ["10 на странице", "20 на странице", "40 на странице"]
                onActivated: root.pageSize = [10, 20, 40][currentIndex]
            }
            Common.AuroraButton {
                uiTheme: theme
                iconOnly: true
                iconName: "previous"
                text: "Предыдущая страница"
                implicitHeight: 34
                enabled: root.pageIndex > 0
                onClicked: root.pageIndex--
            }
            Text {
                text: (root.pageIndex + 1) + " / " + root.pageCount
                color: theme.text
                font.pixelSize: 12
            }
            Common.AuroraButton {
                uiTheme: theme
                iconOnly: true
                iconName: "next"
                text: "Следующая страница"
                implicitHeight: 34
                enabled: root.pageIndex + 1 < root.pageCount
                onClicked: root.pageIndex++
            }
        }
    }
}
