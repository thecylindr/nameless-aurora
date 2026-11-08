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
    readonly property real treeWidth: frameCover ? Math.min(coverRect.width * 1.15, coverRect.y * 1.55) : Math.min(width * 0.84, height * 0.86)
    readonly property real treeHeight: treeWidth * 2 / 3
    readonly property real treeX: frameCover ? coverRect.x + coverRect.width / 2 - treeWidth / 2 : (width - treeWidth) / 2
    readonly property real treeY: frameCover ? coverRect.y - treeHeight + 10 : height * 0.12
    readonly property color leafAccent: Qt.lighter(secondary, 1.35)

    function resetMotion() {
        if (!spring)
            return ;

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
            return ;
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

    opacity: Math.min(1, intensity) * settingsStore.decorationOpacity
    onPrimaryChanged: schedulePaint()
    onSecondaryChanged: schedulePaint()
    onSignatureChanged: schedulePaint()
    onDetailChanged: schedulePaint()
    onMotionActiveChanged: {
        if (!motionActive)
            resetMotion();

    }

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

    Connections {
        function onAppearanceChanged() {
            root.schedulePaint();
        }

        target: settingsStore
    }

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
        scale: settingsStore.objectScale * (1 + root.energy * 0.025 + root.bass * 0.035)

        Canvas {
            id: crown

            function random(value) {
                const number = Math.sin(value * 127.1 + root.signature * 5.7) * 43758.5;
                return number - Math.floor(number);
            }

            function branch(context, points, thickness) {
                context.beginPath();
                context.moveTo(points[0], points[1]);
                context.bezierCurveTo(points[2], points[3], points[4], points[5], points[6], points[7]);
                context.strokeStyle = Qt.rgba(root.primary.r, root.primary.g, root.primary.b, 0.12);
                context.lineWidth = thickness * 3.5;
                context.stroke();
                context.strokeStyle = root.primary;
                context.lineWidth = thickness;
                context.stroke();
                context.strokeStyle = Qt.lighter(root.secondary, 1.6);
                context.lineWidth = Math.max(0.7, thickness * 0.28);
                context.stroke();
            }

            function leaf(context, x, y, radius, angle, tint) {
                context.save();
                context.translate(x, y);
                context.rotate(angle);
                context.scale(1, 0.42);
                context.beginPath();
                context.arc(0, 0, radius, 0, Math.PI * 2);
                context.fillStyle = Qt.rgba(tint.r, tint.g, tint.b, 0.08);
                context.fill();
                context.strokeStyle = Qt.rgba(tint.r, tint.g, tint.b, 0.72);
                context.lineWidth = 0.9;
                context.stroke();
                context.restore();
            }

            objectName: "bonsaiCrown"
            width: 720
            height: 480
            renderStrategy: Canvas.Threaded
            onPaint: {
                const context = getContext("2d");
                context.reset();
                context.lineCap = "round";
                context.lineJoin = "round";
                branch(context, [360, 473, 420, 402, 305, 361, 371, 277], 11);
                branch(context, [371, 277, 401, 226, 351, 208, 383, 147], 6);
                branch(context, [376, 328, 319, 299, 252, 261, 158, 282], 5);
                branch(context, [373, 305, 426, 263, 493, 227, 563, 251], 5);
                branch(context, [372, 262, 327, 211, 268, 194, 224, 199], 4);
                branch(context, [380, 215, 417, 172, 473, 155, 509, 173], 3);
                branch(context, [383, 172, 346, 134, 337, 102, 366, 80], 3);
                const pads = [[156, 263, 94, 32], [245, 188, 100, 39], [365, 92, 104, 43], [480, 166, 106, 38], [567, 241, 83, 34], [287, 295, 71, 27]];
                const count = Math.round((root.detail < 0.6 ? 32 : 58) * (0.6 + settingsStore.objectDensity * 0.7));
                for (let padIndex = 0; padIndex < pads.length; ++padIndex) {
                    const pad = pads[padIndex];
                    for (let index = 0; index < count; ++index) {
                        const key = padIndex * 503 + index * 7;
                        const angle = crown.random(key + 1) * Math.PI * 2;
                        const spread = Math.sqrt(crown.random(key + 2));
                        const x = pad[0] + Math.cos(angle) * spread * pad[2];
                        const y = pad[1] + Math.sin(angle) * spread * pad[3];
                        const tint = index % 3 ? root.secondary : root.primary;
                        crown.leaf(context, x, y, 3 + crown.random(key + 3) * 5, angle * 0.4 - 0.5, tint);
                    }
                }
            }

            transform: Scale {
                xScale: canopy.width / crown.width
                yScale: canopy.height / crown.height
            }

        }

    }

    Repeater {
        // Independent roots emerge from behind the sides and lower edge.
        model: root.frameCover ? 13 : 0

        BonsaiRoot {
            required property int index
            readonly property bool lowerRoot: index >= 8
            readonly property bool rightRoot: !lowerRoot && index >= 4
            readonly property int slot: lowerRoot ? index - 8 : index % 4

            width: root.coverRect.width * 0.17
            height: root.coverRect.height * (0.22 + (index % 3) * 0.045)
            x: (lowerRoot ? root.coverRect.x + root.coverRect.width * (0.1 + slot * 0.2) : rightRoot ? root.coverRect.x + root.coverRect.width - 6 : root.coverRect.x + 6) - width / 2
            y: lowerRoot ? root.coverRect.y + root.coverRect.height - 6 : root.coverRect.y + root.coverRect.height * (0.18 + slot * 0.23)
            rotation: lowerRoot ? (slot - 2) * -8 : (rightRoot ? -1 : 1) * (62 + slot * 4)
            transformOrigin: Item.Top
            accent: index % 3 ? root.secondary : root.primary
            seed: index * 0.618 + root.signature
            bend: root.rootBend
            follow: root.rootFollow
            tip: root.rootTip
            clock: spring.time
            animated: root.motionActive
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
            color: index % 2 ? Qt.lighter(root.secondary, 1.8) : root.leafAccent
            opacity: 0.3 + root.pulse * 0.2 + Math.sin(drift * 2 + index) * 0.12
        }

    }

}
