// Панель загрузок: статус, очередь заданий, прогресс и кнопки управления загрузкой и автоподбором.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../common" as Common

Rectangle {
    id: root
    property bool compact: false
    readonly property bool narrow: width < 620
    clip: true
    property bool expanded: false
    property bool dismissed: false
    readonly property bool collecting: libraryExchange.busy || audioSources.busy || audioSources.recovering || downloadDependencies.busy
    readonly property bool present: collecting || downloadManager.busy || (downloadManager.totalCount > 0 && !dismissed)
    // Сводный статус: по приоритету показывается самая важная фаза (подготовка, перенос, автоподбор, загрузка, итог).
    readonly property string statusText: downloadDependencies.busy ? downloadDependencies.statusText : libraryExchange.busy ? (libraryExchange.status || "Переносим коллекцию…") : audioSources.recovering ? audioSources.recoveryStatus : audioSources.busy ? audioSources.status : downloadManager.busy ? ((downloadManager.currentService || "Музыка") + " · " + downloadManager.currentTitle) : downloadManager.failedCount ? "Часть треков ждёт аудио" : "Готово · " + trackCount(downloadManager.succeededCount) + " в плеере"
    // Высота списка заданий: растёт вместе с содержимым, но ограничена сверху третью окна.
    readonly property real rowsHeight: Math.min(jobs.contentHeight, Math.max(72, Math.min(280, Window.window ? Window.window.height * .32 : 280)))
    implicitHeight: present ? (expanded ? 24 + 44 + 8 + Math.max(downloadManager.totalCount ? 72 : 36, rowsHeight) + 8 + footer.implicitHeight : 68) : 0
    visible: present
    radius: 20
    color: theme.surface
    border.color: Qt.rgba(theme.accent.r, theme.accent.g, theme.accent.b, .22)
    Theme {
        id: theme
    }
    // Склоняет слово «трек» по правилам русского языка (1 трек, 2 трека, 5 треков, 21 трек).
    function trackCount(n) {
        const a = n % 100, b = n % 10;
        return n + " " + (a >= 11 && a <= 14 ? "треков" : b === 1 ? "трек" : b >= 2 && b <= 4 ? "трека" : "треков");
    }
    Rectangle {
        anchors.fill: parent
        anchors.topMargin: 5
        anchors.bottomMargin: -5
        radius: parent.radius
        color: "#30000000"
        z: -1
    }
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onWheel: wheel => wheel.accepted = true
    }
    Connections {
        target: downloadManager
        function onBusyChanged() {
            if (downloadManager.busy)
                root.dismissed = false;
        }
    }
    Connections {
        target: libraryExchange
        function onChanged() {
            if (libraryExchange.busy)
                root.dismissed = false;
        }
    }
    Connections {
        target: audioSources
        function onChanged() {
            if (audioSources.recovering || audioSources.busy)
                root.dismissed = false;
        }
    }
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8
        RowLayout {
            id: head
            Layout.fillWidth: true
            Layout.preferredHeight: 44
            spacing: 12
            Common.ProgressRing {
                Layout.preferredWidth: 42
                Layout.preferredHeight: 42
                color: theme.accent
                trackColor: theme.line
                value: downloadManager.fileTotal > 0 ? downloadManager.filesReady / downloadManager.fileTotal : 0
                indeterminate: downloadManager.fileTotal === 0 && (downloadManager.busy || root.collecting)
            }
            ColumnLayout {
                Layout.fillWidth: true
                Layout.minimumWidth: 0
                spacing: 4
                Text {
                    Layout.fillWidth: true
                    text: root.statusText
                    color: theme.text
                    font.pixelSize: 12
                    font.weight: Font.Medium
                    elide: Text.ElideRight
                }
                RowLayout {
                    Layout.fillWidth: true
                    Layout.minimumWidth: 0
                    spacing: 8
                    Text {
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        elide: Text.ElideRight
                        text: (downloadManager.fileTotal > 0 ? downloadManager.filesReady + " из " + downloadManager.fileTotal + " файлов" : downloadManager.succeededCount + " готово") + (audioSources.recovering ? " · " + audioSources.recoveryPending + " на подборе" : downloadManager.activeCount ? " · " + downloadManager.activeCount + " в очереди" : "") + (downloadManager.failedCount ? " · " + downloadManager.failedCount + " без файла" : "")
                        color: downloadManager.failedCount ? theme.warn : theme.subtext
                        font.pixelSize: 10
                    }
                }
            }
            Common.AuroraButton {
                uiTheme: theme
                text: root.expanded ? "Свернуть" : "Очередь"
                iconName: "queue"
                implicitHeight: 34
                iconOnly: root.narrow
                implicitWidth: root.narrow ? 34 : 118
                onClicked: root.expanded = !root.expanded
            }
            Common.AuroraButton {
                uiTheme: theme
                text: "Другой источник"
                iconName: "search"
                implicitHeight: 34
                iconOnly: root.narrow
                implicitWidth: root.narrow ? 34 : 154
                visible: !root.expanded && downloadManager.firstFailedTrack > 0
                onClicked: audioSources.openTrack(downloadManager.firstFailedTrack)
            }
            Common.AuroraButton {
                uiTheme: theme
                iconOnly: true
                iconName: "close"
                text: "Скрыть завершённые загрузки"
                implicitWidth: 30
                implicitHeight: 30
                visible: !downloadManager.busy && !root.collecting
                onClicked: {
                    root.dismissed = true;
                    root.expanded = false;
                }
            }
        }
        ListView {
            id: jobs
            visible: root.expanded
            Layout.fillWidth: true
            Layout.preferredHeight: root.rowsHeight
            clip: true
            spacing: 6
            model: downloadManager
            ScrollBar.vertical: Common.AuroraScrollBar {
                uiTheme: theme
            }
            delegate: Rectangle {
                id: job
                required property int jobId
                required property int trackId
                required property string title
                required property string service
                required property string status
                required property string errorText
                required property real progress
                width: jobs.width
                height: status === "failed" ? 100 : 72
                radius: 12
                color: theme.surfaceHigh
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 8
                    AlbumArt {
                        Layout.preferredWidth: 40
                        Layout.preferredHeight: 40
                        visible: !root.compact
                        path: job.trackId > 0 ? (libraryModel.trackDetails(job.trackId).coverPath || "") : ""
                        seed: job.title
                        cornerRadius: 10
                    }
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 5
                        Text {
                            Layout.fillWidth: true
                            text: job.title
                            color: theme.text
                            font.pixelSize: 12
                            elide: Text.ElideRight
                        }
                        Text {
                            Layout.fillWidth: true
                            text: job.service + " · " + ({
                                    queued: "В очереди",
                                    resolving: "Получаем аудио…",
                                    downloading: "Скачиваем · " + Math.round(job.progress * 100) + "%",
                                    importing: "Сохраняем",
                                    succeeded: "В плеере",
                                    recovered: "Получен из другого источника",
                                    failed: "Нужен другой источник",
                                    cancelled: "Отменено"
                                }[job.status] || job.status)
                            color: job.status === "failed" ? theme.warn : theme.subtext
                            font.pixelSize: 10
                            elide: Text.ElideRight
                        }
                        Text {
                            Layout.fillWidth: true
                            text: job.errorText
                            visible: job.status === "failed"
                            color: theme.textMuted
                            font.pixelSize: 10
                            maximumLineCount: 2
                            wrapMode: Text.WordWrap
                            elide: Text.ElideRight
                        }
                        Common.AuroraProgress {
                            uiTheme: theme
                            Layout.fillWidth: true
                            value: job.progress
                            indeterminate: job.status === "resolving"
                            visible: ["resolving", "downloading", "importing"].indexOf(job.status) >= 0
                        }
                    }
                    Common.AuroraButton {
                        uiTheme: theme
                        text: "Источник"
                        iconName: "search"
                        implicitHeight: 34
                        iconOnly: root.narrow
                        visible: job.status === "failed" && job.trackId > 0
                        onClicked: audioSources.openTrack(job.trackId)
                    }
                    Common.AuroraButton {
                        uiTheme: theme
                        iconOnly: true
                        iconName: ["failed", "cancelled"].indexOf(job.status) >= 0 ? "refresh" : "close"
                        text: ["failed", "cancelled"].indexOf(job.status) >= 0 ? "Повторить" : "Отменить"
                        implicitWidth: 32
                        implicitHeight: 32
                        visible: ["succeeded", "recovered"].indexOf(job.status) < 0
                        onClicked: ["failed", "cancelled"].indexOf(job.status) >= 0 ? downloadManager.retryJob(job.jobId) : downloadManager.cancelJob(job.jobId)
                    }
                }
            }
        }
        Flow {
            id: footer
            visible: root.expanded
            Layout.fillWidth: true
            spacing: 6
            Common.AuroraButton {
                uiTheme: theme
                text: "Очистить"
                implicitHeight: 32
                onClicked: downloadManager.clearFinished()
            }
            Common.AuroraButton {
                uiTheme: theme
                text: "Открыть отчёты"
                iconOnly: true
                iconName: "folder"
                implicitHeight: 32
                onClicked: downloadManager.openDiagnostics()
            }
            Common.AuroraButton {
                uiTheme: theme
                text: "Автоподбор"
                iconName: "search"
                primary: true
                implicitHeight: 32
                visible: downloadManager.failedCount > 0 && !audioSources.recovering
                onClicked: audioSources.recoverMissing()
            }
            Common.AuroraButton {
                uiTheme: theme
                text: "Стоп"
                implicitHeight: 32
                visible: downloadManager.busy || audioSources.recovering
                onClicked: {
                    audioSources.cancelRecovery();
                    downloadManager.cancelAll();
                }
            }
        }
    }
}
