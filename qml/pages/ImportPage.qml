import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../common" as Common
import "../components" as Components

Item {
    id: root
    property bool compact: false
    property bool authorizedMode: false
    property string authService: "Яндекс Музыка"
    property bool pendingInspect: false
    Theme { id: theme }

    function selectedQuality() {
        return ["original", "balanced", "compact"][qualitySegment.currentIndex]
    }

    function toolReady() {
        return downloadManager.serviceForUrl(urlField.text) === "Spotify"
                ? downloadDependencies.spotdlAvailable : downloadDependencies.ytDlpAvailable
    }

    function inspect() {
        if (!urlField.text.length) {
            toast.show("Вставьте ссылку на трек, плейлист или раздел библиотеки")
            return
        }
        if (authorizedMode && !browserAuth.sessionGranted) {
            consentDialog.open()
            return
        }
        pendingInspect = true
        if (!toolReady()) {
            downloadDependencies.installOrUpdate()
            return
        }
        pendingInspect = false
        importPreviewer.inspect(urlField.text, selectedQuality(), authorizedMode)
    }

    function requestDownload() {
        if (importPreviewer.selectedCount === 0)
            return
        if (importPreviewer.selectedBytes > 0 && !storageManager.canFit(importPreviewer.selectedBytes)) {
            toast.show("Недостаточно места с учётом резерва. Выберите меньше треков или другую папку.")
            return
        }
        if (settingsStore.confirmBulkDownload && importPreviewer.selectedCount > 1)
            confirmDialog.open()
        else
            importPreviewer.startSelected()
    }

    Connections {
        target: downloadDependencies
        function onRuntimeReadyChanged() {
            if (root.pendingInspect && downloadDependencies.runtimeReady) {
                root.pendingInspect = false
                importPreviewer.inspect(urlField.text, root.selectedQuality(), root.authorizedMode)
            }
        }
        function onLastErrorChanged() {
            if (downloadDependencies.lastError.length)
                toast.show(downloadDependencies.lastError)
        }
    }
    Connections {
        target: importPreviewer
        function onLastErrorChanged() {
            if (importPreviewer.lastError.length)
                toast.show(importPreviewer.lastError)
        }
    }

    ScrollView {
        id: scroll
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth
        background: Item {}
        ScrollBar.vertical: Common.AuroraScrollBar { uiTheme: theme; x: scroll.width - width; height: scroll.height }

        ColumnLayout {
            width: Math.min(900, Math.max(280, scroll.availableWidth - (compact ? 32 : 56)))
            x: Math.max(compact ? 16 : 28, (scroll.availableWidth - width) / 2)
            spacing: 18

            Item { implicitHeight: compact ? 16 : 28 }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 3
                Text { text: "Импорт музыки"; color: theme.text; font.pixelSize: compact ? 24 : 30; font.weight: Font.DemiBold }
                Text { text: "Сначала содержимое и объём — потом загрузка"; color: theme.subtext; font.pixelSize: theme.bodySize }
            }

            Common.AuroraSegmented {
                uiTheme: theme
                Layout.fillWidth: true
                model: ["По ссылке", "Моя музыка"]
                currentIndex: authorizedMode ? 1 : 0
                onOptionSelected: index => authorizedMode = index === 1
            }

            Common.AuroraCard {
                uiTheme: theme
                Layout.fillWidth: true
                padding: 18
                implicitHeight: inputContent.implicitHeight + padding * 2
                ColumnLayout {
                    id: inputContent
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 18
                    spacing: 14

                    ColumnLayout {
                        Layout.fillWidth: true
                        visible: authorizedMode
                        spacing: 10
                        Text {
                            Layout.fillWidth: true
                            text: "Войдите в свой аккаунт в системном браузере. Aurora не получает и не сохраняет пароль; разрешение действует только для выбранной операции."
                            color: theme.subtext
                            font.pixelSize: theme.captionSize
                            wrapMode: Text.WordWrap
                        }
                        Common.AuroraSegmented {
                            uiTheme: theme
                            Layout.fillWidth: true
                            model: ["Яндекс Музыка", "VK"]
                            currentIndex: authService === "VK" ? 1 : 0
                            onOptionSelected: index => {
                                authService = index === 1 ? "VK" : "Яндекс Музыка"
                                browserAuth.revokeSessionAccess()
                            }
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            Common.AuroraButton {
                                uiTheme: theme
                                iconName: "link"
                                text: browserAuth.sessionGranted ? "Браузер разрешён" : "Открыть браузер"
                                onClicked: {
                                    browserAuth.openService(authService)
                                    consentDialog.open()
                                }
                            }
                            Common.AuroraButton {
                                uiTheme: theme
                                visible: browserAuth.sessionGranted
                                text: "Отозвать"
                                onClicked: browserAuth.revokeSessionAccess()
                            }
                        }
                    }

                    Common.AuroraField {
                        id: urlField
                        uiTheme: theme
                        Layout.fillWidth: true
                        placeholderText: authorizedMode ? "Ссылка на плейлист или раздел вашей библиотеки" : "https://youtube.com/... или ссылка на другой сервис"
                        iconName: "link"
                        enabled: !importPreviewer.busy
                    }
                    Text {
                        Layout.fillWidth: true
                        text: downloadManager.serviceForUrl(urlField.text).length
                              ? downloadManager.serviceForUrl(urlField.text) + (authorizedMode ? " · авторизованный режим" : "")
                              : "YouTube · Яндекс Музыка · VK · Spotify"
                        color: theme.subtext
                        font.pixelSize: theme.captionSize
                    }
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 7
                        Text { text: "Формат и размер"; color: theme.subtext; font.pixelSize: theme.captionSize }
                        Common.AuroraSegmented {
                            id: qualitySegment
                            uiTheme: theme
                            Layout.fillWidth: true
                            model: ["Оригинал", "192 kbps", "128 kbps"]
                            currentIndex: ["original", "balanced", "compact"].indexOf(settingsStore.quality)
                            onOptionSelected: index => settingsStore.quality = ["original", "balanced", "compact"][index]
                        }
                    }
                    Common.AuroraButton {
                        uiTheme: theme
                        Layout.fillWidth: true
                        primary: true
                        iconName: "search"
                        text: importPreviewer.busy ? "Получаем список…" : downloadDependencies.busy ? "Устанавливаем компоненты…" : "Показать содержимое"
                        enabled: !importPreviewer.busy && !downloadDependencies.busy
                        onClicked: root.inspect()
                    }
                    Common.AuroraProgress {
                        uiTheme: theme
                        Layout.fillWidth: true
                        visible: importPreviewer.busy || downloadDependencies.busy
                        indeterminate: true
                    }
                    Text {
                        Layout.fillWidth: true
                        visible: text.length > 0
                        text: importPreviewer.lastError.length ? importPreviewer.lastError
                              : downloadDependencies.busy ? downloadDependencies.statusText : importPreviewer.statusText
                        color: importPreviewer.lastError.length ? theme.danger : theme.subtext
                        font.pixelSize: theme.captionSize
                        wrapMode: Text.WordWrap
                    }
                }
            }

            Common.AuroraCard {
                uiTheme: theme
                Layout.fillWidth: true
                visible: previewList.count > 0
                padding: 16
                implicitHeight: previewColumn.implicitHeight + padding * 2
                ColumnLayout {
                    id: previewColumn
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 16
                    spacing: 12
                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            Layout.fillWidth: true
                            text: importPreviewer.selectedCount + " из " + previewList.count + " выбрано"
                            color: theme.text
                            font.pixelSize: theme.bodySize
                            font.weight: Font.Medium
                        }
                        Common.AuroraButton {
                            uiTheme: theme
                            text: importPreviewer.selectedCount === previewList.count ? "Снять выбор" : "Выбрать все"
                            implicitHeight: 38
                            onClicked: importPreviewer.selectAll(importPreviewer.selectedCount !== previewList.count)
                        }
                    }
                    ListView {
                        id: previewList
                        Layout.fillWidth: true
                        implicitHeight: Math.min(320, contentHeight)
                        model: importPreviewer
                        clip: true
                        spacing: 4
                        interactive: contentHeight > height
                        delegate: Rectangle {
                            required property int index
                            required property string title
                            required property string artist
                            required property var duration
                            required property var size
                            required property bool selected
                            width: previewList.width
                            height: 56
                            radius: theme.radiusSmall
                            color: selected ? theme.selected : theme.surfaceHigh
                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 8
                                anchors.rightMargin: 12
                                spacing: 10
                                Common.AuroraButton {
                                    uiTheme: theme
                                    iconOnly: true
                                    iconName: selected ? "check" : "plus"
                                    implicitWidth: 36
                                    implicitHeight: 36
                                    primary: selected
                                    onClicked: importPreviewer.toggle(index)
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 2
                                    Text { Layout.fillWidth: true; text: title; color: theme.text; font.pixelSize: theme.bodySize; elide: Text.ElideRight }
                                    Text { Layout.fillWidth: true; text: artist; color: theme.subtext; font.pixelSize: theme.captionSize; elide: Text.ElideRight }
                                }
                                Text {
                                    text: duration > 0 ? Math.floor(duration / 60) + ":" + String(Math.floor(duration % 60)).padStart(2, "0") : "—"
                                    color: theme.subtext
                                    font.pixelSize: theme.captionSize
                                }
                            }
                        }
                    }
                    Text {
                        Layout.fillWidth: true
                        text: "Примерный объём: " + storageManager.formatBytes(importPreviewer.selectedBytes)
                              + (importPreviewer.unknownSizes > 0 ? " · размер " + importPreviewer.unknownSizes + " элементов неизвестен" : "")
                        color: theme.subtext
                        font.pixelSize: theme.captionSize
                        wrapMode: Text.WordWrap
                    }
                    Components.StorageSummary { Layout.fillWidth: true; requiredBytes: importPreviewer.selectedBytes }
                    Common.AuroraButton {
                        uiTheme: theme
                        Layout.fillWidth: true
                        primary: true
                        iconName: "download"
                        text: "Загрузить выбранное · " + storageManager.formatBytes(importPreviewer.selectedBytes)
                        enabled: importPreviewer.selectedCount > 0
                        onClicked: root.requestDownload()
                    }
                }
            }

            Common.AuroraCard {
                uiTheme: theme
                Layout.fillWidth: true
                visible: queueList.count > 0
                padding: 16
                implicitHeight: queueColumn.implicitHeight + padding * 2
                ColumnLayout {
                    id: queueColumn
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 16
                    spacing: 12
                    RowLayout {
                        Layout.fillWidth: true
                        Text { Layout.fillWidth: true; text: "Очередь · " + downloadManager.activeCount + " активных"; color: theme.text; font.pixelSize: theme.bodySize; font.weight: Font.Medium }
                        Common.AuroraButton { uiTheme: theme; text: "Очистить"; implicitHeight: 36; onClicked: downloadManager.clearFinished() }
                    }
                    Common.AuroraProgress { uiTheme: theme; Layout.fillWidth: true; from: 0; to: 1; value: downloadManager.progress }
                    ListView {
                        id: queueList
                        Layout.fillWidth: true
                        implicitHeight: Math.min(280, contentHeight)
                        model: downloadManager
                        clip: true
                        interactive: contentHeight > height
                        delegate: Rectangle {
                            required property int jobId
                            required property string title
                            required property string url
                            required property string status
                            required property double progress
                            width: queueList.width
                            height: 64
                            color: "transparent"
                            RowLayout {
                                anchors.fill: parent
                                spacing: 10
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 4
                                    Text { Layout.fillWidth: true; text: title.length ? title : url; color: theme.text; font.pixelSize: theme.captionSize; elide: Text.ElideMiddle }
                                    Common.AuroraProgress { uiTheme: theme; Layout.fillWidth: true; from: 0; to: 1; value: progress; visible: status === "downloading" || status === "importing" }
                                    Text { Layout.fillWidth: true; text: status; color: status === "failed" ? theme.danger : theme.subtext; font.pixelSize: 10; elide: Text.ElideRight }
                                }
                                Common.AuroraButton {
                                    uiTheme: theme
                                    iconOnly: true
                                    iconName: status === "failed" || status === "cancelled" ? "refresh" : "close"
                                    implicitWidth: 36
                                    implicitHeight: 36
                                    visible: status !== "succeeded"
                                    onClicked: status === "failed" || status === "cancelled" ? downloadManager.retryJob(jobId) : downloadManager.cancelJob(jobId)
                                }
                            }
                        }
                    }
                }
            }
            Item { implicitHeight: 28 }
        }
    }

    Common.AuroraDialog {
        id: consentDialog
        uiTheme: theme
        title: "Доступ к браузерной сессии"
        modal: true
        width: Math.min(root.width - 32, 500)
        contentItem: ColumnLayout {
            spacing: 16
            Text {
                Layout.fillWidth: true
                text: browserAuth.consentText(authService)
                color: theme.text
                font.pixelSize: theme.bodySize
                wrapMode: Text.WordWrap
            }
            Text {
                Layout.fillWidth: true
                text: "После входа выберите плейлист или раздел библиотеки. DRM, защищённый кэш и ограничения сервиса не обходятся."
                color: theme.subtext
                font.pixelSize: theme.captionSize
                wrapMode: Text.WordWrap
            }
            RowLayout {
                Layout.fillWidth: true
                Common.AuroraButton { uiTheme: theme; Layout.fillWidth: true; text: "Отмена"; onClicked: consentDialog.close() }
                Common.AuroraButton {
                    uiTheme: theme
                    primary: true
                    Layout.fillWidth: true
                    text: "Разрешить один раз"
                    onClicked: {
                        browserAuth.grantSessionAccess()
                        consentDialog.close()
                    }
                }
            }
        }
    }

    Common.AuroraDialog {
        id: confirmDialog
        uiTheme: theme
        title: "Подтвердите загрузку"
        modal: true
        width: Math.min(root.width - 32, 500)
        contentItem: ColumnLayout {
            spacing: 16
            Text {
                Layout.fillWidth: true
                text: importPreviewer.selectedCount + " треков · примерно " + storageManager.formatBytes(importPreviewer.selectedBytes)
                color: theme.text
                font.pixelSize: 20
                font.weight: Font.DemiBold
            }
            Text {
                Layout.fillWidth: true
                text: "Свободно: " + storageManager.formatBytes(storageManager.freeBytes)
                      + (importPreviewer.unknownSizes ? "\nДля части элементов объём будет известен только во время загрузки." : "")
                color: theme.subtext
                font.pixelSize: theme.bodySize
                wrapMode: Text.WordWrap
            }
            RowLayout {
                Layout.fillWidth: true
                Common.AuroraButton { uiTheme: theme; Layout.fillWidth: true; text: "Отмена"; onClicked: confirmDialog.close() }
                Common.AuroraButton {
                    uiTheme: theme
                    primary: true
                    Layout.fillWidth: true
                    text: "Загрузить"
                    onClicked: {
                        confirmDialog.close()
                        importPreviewer.startSelected()
                    }
                }
            }
        }
    }

    Common.AuroraToast {
        id: toast
        uiTheme: theme
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        width: Math.min(parent.width - 32, 520)
    }
}
