// Базовый класс эффектов: управляет видимостью, паузами, качеством и тактом анимации в зависимости от загрузки.
import QtQuick

Item {
    id: root

    property bool animated: true
    property bool motionEnabled: true
    property bool lively: false

    readonly property bool hardwareAccelerated: GraphicsInfo.api !== GraphicsInfo.Software && GraphicsInfo.api !== GraphicsInfo.Unknown
    readonly property bool mobile: Qt.platform.os === "android" || Qt.platform.os === "ios"
    readonly property bool applicationVisible: Application.state !== Qt.ApplicationSuspended && Application.state !== Qt.ApplicationHidden
    readonly property bool windowVisible: applicationVisible && Window.window !== null && Window.window.visibility !== Window.Hidden && Window.window.visibility !== Window.Minimized
    // Эффект «показывается», только когда реально видим на экране и в пределах окна.
    readonly property bool presented: visible && opacity > 0 && width > 0 && height > 0 && windowVisible && timing.inViewport

    // Движение запускается лишь для видимых эффектов с включённой анимацией в настройках.
    readonly property bool motionRunning: presented && animated && motionEnabled && settingsStore.motionEnabled
    readonly property real detail: hardwareAccelerated ? (mobile ? 0.75 : 1) * timing.quality : 0.45
    readonly property int textureLimit: Math.round((mobile ? 640 : 960) * timing.quality)

    signal tick(real seconds)

    // Проверяем и обрезающих предков: visible остаётся true, когда Flickable уносит элемент за пределы прокрутки.
    function updateVisibility() {
        if (!timing)
            return;
        let inViewport = width > 0 && height > 0;
        for (let item = root; item && inViewport; item = item.parent) {
            if (!item.visible || item.opacity <= 0) {
                inViewport = false;
            } else if (item.clip) {
                const bounds = root.mapToItem(item, 0, 0, root.width, root.height);
                inViewport = bounds.x < item.width && bounds.y < item.height && bounds.x + bounds.width > 0 && bounds.y + bounds.height > 0;
            }
        }
        timing.inViewport = inViewport;
    }

    onVisibleChanged: updateVisibility()
    onWidthChanged: updateVisibility()
    onHeightChanged: updateVisibility()
    onMotionRunningChanged: {
        timing.previousTime = 0;
        timing.sampleTime = 0;
        timing.recoveryTime = 0;
    }
    Component.onCompleted: Qt.callLater(updateVisibility)

    QtObject {
        id: timing

        property bool inViewport: false
        property double previousTime: 0
        property real quality: 1
        property real sampleTime: 0
        property real recoveryTime: 0
    }

    Timer {
        interval: 250
        repeat: true
        running: root.visible && root.opacity > 0 && root.windowVisible && root.animated && root.motionEnabled && settingsStore.motionEnabled
        onTriggered: root.updateVisibility()
    }

    FrameAnimation {
        running: root.motionRunning && root.hardwareAccelerated && root.lively

        onTriggered: {
            // Ограничиваем паузы и длительные зависания, чтобы сцена не «прыгала» через экран.
            const seconds = Math.max(0, Math.min(frameTime, 0.1));
            root.tick(seconds);
            timing.sampleTime += seconds;
            if (timing.sampleTime < 2)
                return;

            // Адаптивное качество: при просадке кадров снижаем детализацию, при стабильности — постепенно восстанавливаем.
            const budget = 1 / 60;
            if (smoothFrameTime > budget * 1.45) {
                timing.quality = Math.max(0.5, timing.quality - 0.15);
                timing.recoveryTime = 0;
            } else if (smoothFrameTime < budget * 1.1) {
                timing.recoveryTime += timing.sampleTime;
                if (timing.recoveryTime >= 8) {
                    timing.quality = Math.min(1, timing.quality + 0.1);
                    timing.recoveryTime = 0;
                }
            } else {
                timing.recoveryTime = 0;
            }
            timing.sampleTime = 0;
        }
    }

    // Программному рендеру выгоднее реже просыпаться и держать меньше объектов.
    Timer {
        interval: root.lively ? 50 : 100
        repeat: true
        running: root.motionRunning && (!root.hardwareAccelerated || !root.lively)

        onTriggered: {
            const now = Date.now();
            const seconds = timing.previousTime > 0 ? (now - timing.previousTime) / 1000 : interval / 1000;
            timing.previousTime = now;
            root.tick(Math.max(0, Math.min(seconds, 0.1)));
        }
    }
}
