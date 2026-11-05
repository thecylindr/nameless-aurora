// Отдельное окно встроенного браузера для входа в музыкальные сервисы.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtWebEngine
import ".."
import "../common" as Common

Window {
    id: root
    width: 1080
    height: 800
    minimumWidth: 400
    minimumHeight: 500
    visible: true
    title: webLogin.nativeService + " · Aurora"
    color: theme.background
    Theme {
        id: theme
    }
    onClosing: webLogin.setBrowserVisible(false)
    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 76
            color: theme.surface
            RowLayout {
                anchors.fill: parent
                anchors.margins: 14
                spacing: 12
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 5
                    Text {
                        text: webLogin.nativeService
                        color: theme.text
                        font.pixelSize: 17
                        font.weight: Font.DemiBold
                    }
                    Text {
                        Layout.fillWidth: true
                        text: webLogin.status || "Аккаунт сервиса внутри Aurora"
                        color: theme.subtext
                        font.pixelSize: 12
                        elide: Text.ElideRight
                    }
                }
                Common.AuroraButton {
                    uiTheme: theme
                    iconOnly: true
                    iconName: "refresh"
                    text: "Обновить страницу"
                    onClicked: browser.reload()
                }
                Common.AuroraButton {
                    uiTheme: theme
                    primary: true
                    visible: webLogin.awaitingLogin
                    text: "Я вошёл"
                    onClicked: webLogin.confirmNativeLogin()
                }
                Common.AuroraButton {
                    uiTheme: theme
                    text: "Закрыть"
                    onClicked: webLogin.setBrowserVisible(false)
                }
            }
        }
        Common.AuroraProgress {
            uiTheme: theme
            Layout.fillWidth: true
            visible: browser.loading
            from: 0
            to: 100
            value: browser.loadProgress
        }
        WebEngineView {
            id: browser
            Layout.fillWidth: true
            Layout.fillHeight: true
            profile: webLogin.nativeProfile
            url: webLogin.pageUrl
            audioMuted: true
            settings.playbackRequiresUserGesture: false
            settings.javascriptCanOpenWindows: true
            // Сообщаем логике входа, загрузилась ли страница, чтобы показать кнопку «Я вошёл» или ошибку.
            onLoadingChanged: info => {
                if (info.status === WebEngineView.LoadSucceededStatus)
                    webLogin.pageLoaded(url, true);
                else if (info.status === WebEngineView.LoadFailedStatus)
                    webLogin.pageLoaded(url, false, "Сайт не открылся. Проверь соединение и повтори попытку.");
            }
            onNewWindowRequested: request => request.openIn(browser)
            onRenderProcessTerminated: (status, code) => webLogin.pageLoaded(url, false, "Веб-окно остановилось. Обнови страницу и повтори операцию.")
        }
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: webLogin.pageError ? 68 : 34
            color: theme.surface
            Text {
                anchors.fill: parent
                anchors.margins: 10
                text: webLogin.pageError || "Вход выполняется на сайте сервиса. Отмена доступна в любой момент."
                color: webLogin.pageError ? theme.warn : theme.subtext
                font.pixelSize: 12
                wrapMode: Text.WordWrap
            }
        }
    }
    // Выполняем JS по запросу логики входа и перенаправляем браузер на нужную страницу.
    Connections {
        target: webLogin
        function onScriptRequested(id, script) {
            browser.runJavaScript(script, result => webLogin.submitScriptResult(id, result));
        }
        function onNavigateRequested(url) {
            if (browser.url.toString() === url.toString())
                browser.reload();
            else
                browser.url = url;
        }
    }
}
