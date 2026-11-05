import QtQuick

Item {
    id: root

    property color primary: "#baa5ff"
    property color secondary: "#83dbc7"
    property real signature: 0.5
    property real phase: 0
    property real energy: 0
    property real bass: 0
    property real pulse: 0
    property real intensity: 0.85
    property real detail: 1
    property bool animated: true
    property bool frameCover: false
    property rect coverRect: Qt.rect(width * 0.23, height * 0.43, width * 0.54, height * 0.49)

    readonly property bool motionActive: animated && visible && intensity > 0 && settingsStore.motionEnabled
    readonly property real rootBend: spring.position * 4
    readonly property real rootFollow: spring.follow * 4
    readonly property real rootTip: spring.tip * 4
    readonly property real bassDrive: motionActive ? Math.max(0, Math.min(1, (bass - 0.25) / 0.9)) : 0
    readonly property real tremor: bassDrive * 0.85 + (motionActive ? Math.min(1, pulse) * 0.3 : 0)
    readonly property real treeWidth: frameCover ? coverRect.width * 1.42 : Math.min(width * 0.96, height * 1.24)
    readonly property real treeHeight: treeWidth * 0.47
    readonly property real treeX: frameCover ? coverRect.x + coverRect.width / 2 - treeWidth / 2 : (width - treeWidth) / 2
    readonly property real treeY: frameCover ? coverRect.y - coverRect.width * 0.57 : height * 0.12
    readonly property color leafAccent: Qt.rgba(0.48 + secondary.r * 0.15, 0.73 + secondary.g * 0.12, 0.32 + secondary.b * 0.14, 1)

    opacity: Math.min(1, intensity) * settingsStore.decorationOpacity

    QtObject {
        id: spring
        property real position: 0
        property real velocity: 0
        property real follow: 0
        property real tip: 0
        property real previousBass: 0
        property real previousPulse: 0
        property real time: 0
    }

    function resetMotion() {
        if (!spring)
            return;
        spring.position = 0;
        spring.velocity = 0;
        spring.follow = 0;
        spring.tip = 0;
        spring.previousBass = 0;
        spring.previousPulse = 0;
        spring.time = 0;
    }

    // The atmosphere's clock advances this spring; painting stays independent of audio.
    function advance(seconds) {
        if (!motionActive) {
            resetMotion();
            return;
        }
        const elapsed = Math.max(0, Math.min(0.1, seconds));
        const bassRise = Math.max(0, bass - spring.previousBass - 0.035);
        const pulseRise = Math.max(0, pulse - spring.previousPulse - 0.04);
        spring.previousBass = bass;
        spring.previousPulse = pulse;
        spring.velocity = Math.min(14, spring.velocity + bassRise * 7 + pulseRise * 3);

        // Small substeps keep the spring stable on slow frames and after a stalled frame.
        const steps = Math.max(1, Math.ceil(elapsed / 0.008));
        const step = elapsed / steps;
        for (let index = 0; index < steps; ++index) {
            const acceleration = (bassDrive * 0.35 - spring.position) * 105 - spring.velocity * 10;
            spring.velocity += acceleration * step;
            spring.position = Math.max(-1.8, Math.min(1.8, spring.position + spring.velocity * step));
            spring.follow += (spring.position - spring.follow) * (1 - Math.exp(-step / 0.08));
            spring.tip += (spring.follow - spring.tip) * (1 - Math.exp(-step / 0.13));
        }
        spring.time += elapsed;
    }

    function schedulePaint() {
        if (repaintDelay && !repaintDelay.running)
            repaintDelay.start();
    }

    onPrimaryChanged: schedulePaint()
    onSecondaryChanged: schedulePaint()
    onSignatureChanged: schedulePaint()
    onDetailChanged: schedulePaint()
    onMotionActiveChanged: if (!motionActive) resetMotion()

    Timer {
        id: repaintDelay
        interval: 100
        onTriggered: crown.requestPaint()
    }

    Item {
        id: canopy
        x: root.treeX
        y: root.treeY
        width: root.treeWidth
        height: root.treeHeight
        transformOrigin: Item.Bottom
        rotation: root.motionActive ? Math.sin(root.phase * 0.7) * 0.35 + root.rootBend * 0.08 : 0

        Canvas {
            id: crown
            objectName: "bonsaiCrown"
            width: 768
            height: 360
            renderStrategy: Canvas.Threaded
            transform: Scale { xScale: canopy.width / crown.width; yScale: canopy.height / crown.height }

            onPaint: {
                const context = getContext("2d");
                context.reset();
                const seed = root.signature;
                function random(value) {
                    const result = Math.sin(value * 127.1 + seed * 5.7) * 43758.5453;
                    return result - Math.floor(result);
                }
                function oval(x, y, rx, ry, color) {
                    context.save();
                    context.translate(x, y);
                    context.scale(1, ry / rx);
                    context.beginPath();
                    context.arc(0, 0, rx, 0, Math.PI * 2);
                    context.fillStyle = color;
                    context.fill();
                    context.restore();
                }
                function branch(points, width) {
                    context.beginPath();
                    context.moveTo(points[0], points[1]);
                    context.bezierCurveTo(points[2], points[3], points[4], points[5], points[6], points[7]);
                    context.lineWidth = width;
                    context.strokeStyle = "#70492f";
                    context.stroke();
                    context.lineWidth = Math.max(1.5, width * 0.3);
                    context.strokeStyle = "#c39d62";
                    context.stroke();
                }
                const halo = context.createRadialGradient(390, 155, 50, 390, 160, 330);
                halo.addColorStop(0, Qt.rgba(root.secondary.r, root.secondary.g, root.secondary.b, 0.16));
                halo.addColorStop(1, "transparent");
                context.fillStyle = halo;
                context.fillRect(0, 0, 768, 360);

                const pads = [
                    [165, 195, 108, 48], [257, 119, 126, 52], [390, 83, 142, 56],
                    [527, 128, 136, 56], [635, 194, 88, 43], [475, 219, 131, 53],
                    [302, 212, 124, 49], [115, 234, 78, 30], [566, 243, 83, 31]
                ];
                for (const pad of pads)
                    oval(pad[0], pad[1] + 9, pad[2], pad[3], "#123b29");

                context.lineCap = "round";
                context.lineJoin = "round";
                branch([385, 348, 426, 294, 352, 264, 377, 214], 43);
                branch([377, 220, 395, 179, 358, 152, 392, 93], 24);
                branch([382, 253, 436, 227, 488, 183, 540, 144], 21);
                branch([383, 262, 330, 224, 284, 226, 220, 174], 20);
                branch([410, 222, 451, 208, 495, 230, 553, 223], 13);
                branch([359, 211, 329, 184, 298, 149, 262, 133], 11);
                branch([375, 330, 308, 338, 248, 328, 116, 355], 18);
                branch([396, 330, 460, 337, 544, 326, 652, 355], 18);

                const palette = ["#245d36", "#397f43", "#57994b", "#74ae52", "#a0c76c"];
                const leaves = root.detail < 0.6 ? 42 : 68;
                for (let padIndex = 0; padIndex < pads.length; ++padIndex) {
                    const pad = pads[padIndex];
                    const shade = context.createLinearGradient(0, pad[1] - pad[3], 0, pad[1] + pad[3]);
                    shade.addColorStop(0, "#5d9848");
                    shade.addColorStop(0.5, "#38763b");
                    shade.addColorStop(1, "#18472d");
                    oval(pad[0], pad[1], pad[2] * 0.94, pad[3] * 0.91, shade);
                    for (let leaf = 0; leaf < leaves; ++leaf) {
                        const key = padIndex * 153 + leaf * 7;
                        const angle = random(key + 1) * Math.PI * 2;
                        const spread = Math.sqrt(random(key + 2));
                        const x = pad[0] + Math.cos(angle) * spread * pad[2];
                        const y = pad[1] + Math.sin(angle) * spread * pad[3];
                        const radius = 3 + random(key + 3) * 7;
                        const colorIndex = Math.min(4, Math.floor(random(key + 4) * 3 + (1 - spread) * 2));
                        oval(x, y, radius * 1.5, radius * 0.62, palette[colorIndex]);
                    }
                }
                // A few warm tips give the dense canopy depth without a blur pass.
                for (let tip = 0; tip < 28; ++tip) {
                    const pad = pads[tip % pads.length];
                    oval(pad[0] + (random(tip + 502) - 0.5) * pad[2] * 1.6,
                         pad[1] - pad[3] * 0.45 + random(tip + 601) * 12, 3, 1.5, "#d0d58a");
                }
            }
        }
    }

    Repeater {
        model: root.frameCover ? 2 : 0

        BonsaiRoot {
            required property int index
            objectName: index ? "bonsaiRightRoot" : "bonsaiLeftRoot"
            side: index ? 1 : -1
            width: root.coverRect.width * 0.22
            height: root.coverRect.height * 1.03
            x: (index ? root.coverRect.x + root.coverRect.width : root.coverRect.x) - width / 2
            y: root.coverRect.y + root.coverRect.height * 0.035
            accent: root.leafAccent
            bend: root.rootBend
            follow: root.rootFollow
            tip: root.rootTip
            tremor: root.tremor
            clock: spring.time
        }
    }

    Repeater {
        model: settingsStore.objectDensity > 0 ? Math.max(2, Math.round(6 * root.detail)) : 0

        Rectangle {
            required property int index
            readonly property real drift: root.motionActive ? root.phase * (0.12 + index * 0.02) : 0
            width: 2 + index % 2
            height: width
            radius: width / 2
            x: root.treeX + root.treeWidth * (0.14 + (index * 0.618 % 1) * 0.72) + Math.sin(drift + index) * 5
            y: root.treeY + root.treeHeight * (0.18 + (index * 0.382 % 1) * 0.75)
            color: index % 2 ? "#d0d58a" : root.leafAccent
            opacity: 0.3 + root.pulse * 0.2 + Math.sin(drift * 2 + index) * 0.12
        }
    }
}
