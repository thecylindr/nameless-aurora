// Главная «сфера» атмосферы: шейдерный шар с кольцами и реакцией на бас, плюс программный запасной вариант.
import QtQuick
import ".."

EffectSurface {
    id: root

    property color primary: player.hasTrack ? player.atmosphereColor : theme.accent
    property color secondary: player.hasTrack ? player.atmosphereSecondary : theme.accent2
    property real signature: player.hasTrack ? player.atmosphereShape : 0.5
    property string style: settingsStore.sphereStyle
    property real intensity: settingsStore.atmosphereIntensity
    property real phase: 0
    property bool interactive: true
    property real pointerX: 0
    property real pointerY: 0
    property real shockProgress: 1
    property real hitStrength: 0
    property double lastBeatSerial: 0
    property real energy: 0
    property real bass: 0
    property real pulse: 0

    readonly property bool sphereVisible: settingsStore.atmosphereScene === "orb" || settingsStore.atmosphereScene === "cosmos"
    readonly property bool reacting: motionRunning && player.playing
    readonly property real pixelRatio: Screen.devicePixelRatio
    // Насколько уменьшаем внутреннюю текстуру шейдера, чтобы не перегружать слабые GPU.
    readonly property real renderScale: Math.min(1, textureLimit / Math.max(1, width * pixelRatio, height * pixelRatio))

    implicitWidth: 360
    implicitHeight: width
    animated: settingsStore.motionEnabled
    lively: player.playing
    motionEnabled: intensity > 0 && settingsStore.atmosphereScene !== "off" && (sphereVisible || settingsStore.atmosphereScene === "bonsai" || settingsStore.objectDensity > 0)

    Behavior on primary {
        enabled: root.motionRunning
        ColorAnimation {
            duration: 1000
        }
    }
    Behavior on secondary {
        enabled: root.motionRunning
        ColorAnimation {
            duration: 1200
        }
    }
    Behavior on signature {
        enabled: root.motionRunning
        NumberAnimation {
            duration: 1000
            easing.type: Easing.InOutCubic
        }
    }

    onReactingChanged: {
        if (!reacting) {
            energy = 0;
            bass = 0;
            pulse = 0;
            shockProgress = 1;
            hitStrength = 0;
        }
    }
    // Экспоненциально сглаживает энергию, бас и пульс, накапливает фазу и спад ударной волны.
    onTick: seconds => {
        const strength = settingsStore.reactionStrength;
        const targetEnergy = reacting ? Math.min(1.5, player.audioEnergy * strength) : 0;
        const targetBass = reacting ? Math.min(1.5, player.bassEnergy * strength) : 0;
        const targetPulse = reacting && settingsStore.beatEffects ? Math.min(1.5, player.beatPulse * strength) : 0;
        energy += (targetEnergy - energy) * (1 - Math.exp(-seconds / 0.09));
        bass += (targetBass - bass) * (1 - Math.exp(-seconds / 0.065));
        pulse += (targetPulse - pulse) * (1 - Math.exp(-seconds / 0.035));
        phase += seconds * settingsStore.effectSpeed * (0.4 + energy * 0.8 + pulse * 0.5);
        shockProgress = Math.min(1, shockProgress + seconds / 0.76);
    }

    Theme {
        id: theme
    }

    Connections {
        target: player

        // На каждом ударе запускает ударную волну и сбрасывает её прогресс.
        function onAudioReactivityChanged() {
            const serial = player.beatSerial;
            if (serial > root.lastBeatSerial && root.reacting && settingsStore.beatEffects) {
                root.hitStrength = Math.min(1.4, Math.max(0.4, player.beatPulse) * settingsStore.reactionStrength);
                root.shockProgress = 0;
            }
            root.lastBeatSerial = serial;
        }
    }

    Loader {
        anchors.fill: parent
        active: root.hardwareAccelerated && root.presented && root.sphereVisible && root.intensity > 0

        sourceComponent: ShaderEffect {
            objectName: "auroraOrbShader"
            readonly property bool validShader: status === ShaderEffect.Compiled
            property real phase: root.phase
            property real intensity: root.intensity
            // Числовой вариант стиля для шейдера: 0 — ленты, 1 — орбиты, 2 — поток.
            property real variant: root.style === "orbit" ? 1 : root.style === "liquid" ? 2 : 0
            property real energy: root.energy
            property real bass: root.bass
            property real pulse: root.pulse
            property real signature: root.signature
            property real shock: root.shockProgress
            property real impact: root.reacting && settingsStore.beatEffects ? root.hitStrength : 0
            property color colorA: root.primary
            property color colorB: root.secondary
            property vector2d resolution: Qt.vector2d(width, height)
            property vector2d pointer: Qt.vector2d(root.pointerX, root.pointerY)

            fragmentShader: "qrc:/aurora/shaders/aurora-orb.frag.qsb"
            // Ограничиваем работу фрагментного шейдера на крупных экранах с высокой плотностью пикселей; маленькие сферы рисуются напрямую.
            layer.enabled: root.renderScale < 1
            layer.textureSize: Qt.size(Math.max(1, Math.round(width * root.pixelRatio * root.renderScale)), Math.max(1, Math.round(height * root.pixelRatio * root.renderScale)))
            layer.smooth: true
        }
    }

    // Программный запасной вариант: та же сфера рисуется на Canvas, когда нет аппаратного ускорения.
    Loader {
        anchors.fill: parent
        active: !root.hardwareAccelerated && root.presented && root.sphereVisible && root.intensity > 0

        sourceComponent: Item {
            Canvas {
                id: cached
                width: 256
                height: 256
                anchors.centerIn: parent
                scale: Math.min(root.width, root.height) / 256 * (1 + root.bass * 0.055 + root.pulse * 0.045)
                rotation: Math.sin(root.phase * 0.23 + root.signature * 5) * (3 + root.energy * 5)
                opacity: 0.84 + Math.min(0.16, root.energy * 0.08 + root.pulse * 0.08)
                renderStrategy: Canvas.Threaded
                transform: [
                    Scale {
                        origin.x: 128
                        origin.y: 128
                        xScale: 1 + Math.sin(root.phase * 0.7 + root.signature * 6) * root.energy * 0.075
                        yScale: 1 - Math.sin(root.phase * 0.7 + root.signature * 6) * root.energy * 0.06
                    },
                    Translate {
                        x: root.pointerX * 2
                        y: root.pointerY * 2
                    }
                ]

                Timer {
                    id: repaintDelay
                    interval: 100
                    onTriggered: if (root.presented)
                        cached.requestPaint()
                }
                Connections {
                    target: root
                    function onPrimaryChanged() {
                        if (!repaintDelay.running)
                            repaintDelay.start();
                    }
                    function onSecondaryChanged() {
                        if (!repaintDelay.running)
                            repaintDelay.start();
                    }
                    function onStyleChanged() {
                        if (!repaintDelay.running)
                            repaintDelay.start();
                    }
                    function onIntensityChanged() {
                        if (!repaintDelay.running)
                            repaintDelay.start();
                    }
                    function onSignatureChanged() {
                        if (!repaintDelay.running)
                            repaintDelay.start();
                    }
                }

                onPaint: {
                    const context = getContext("2d");
                    context.reset();
                    const x = 128, y = 128, radius = 81;
                    const primary = root.primary, secondary = root.secondary, form = root.signature;
                    function rgba(color, alpha) {
                        return Qt.rgba(color.r, color.g, color.b, alpha);
                    }

                    const halo = context.createRadialGradient(x, y, radius * 0.65, x, y, radius * 1.52);
                    halo.addColorStop(0, rgba(primary, 0.3 * root.intensity));
                    halo.addColorStop(1, "transparent");
                    context.fillStyle = halo;
                    context.fillRect(0, 0, 256, 256);

                    context.save();
                    context.translate(x, y);
                    context.rotate(-0.55 + form * 0.4);
                    context.scale(1, 0.32);
                    for (let ring = 0; ring < (root.style === "orbit" ? 4 : 2); ++ring) {
                        context.beginPath();
                        context.arc(0, 0, radius * (1.32 + ring * 0.08), 0, Math.PI * 2);
                        context.strokeStyle = rgba(ring % 2 ? secondary : primary, 0.22 / (ring + 1));
                        context.lineWidth = 1.2;
                        context.stroke();
                    }
                    context.restore();
                    context.save();
                    context.beginPath();
                    for (let point = 0; point <= 100; ++point) {
                        const angle = point / 100 * Math.PI * 2;
                        const r = radius * (1 + Math.sin(angle * (form > 0.5 ? 4 : 3) + form * 6) * (root.style === "liquid" ? 0.065 : 0.03));
                        const px = x + Math.cos(angle) * r, py = y + Math.sin(angle) * r;
                        if (point)
                            context.lineTo(px, py);
                        else
                            context.moveTo(px, py);
                    }
                    context.closePath();
                    context.clip();

                    const body = context.createRadialGradient(x - radius * 0.4, y - radius * 0.5, 0, x, y, radius * 1.2);
                    body.addColorStop(0, Qt.lighter(primary, 1.16));
                    body.addColorStop(0.42, Qt.darker(primary, 1.7));
                    body.addColorStop(0.78, Qt.darker(secondary, 2.6));
                    body.addColorStop(1, "#07111c");
                    context.fillStyle = body;
                    context.fillRect(x - radius * 1.1, y - radius * 1.1, radius * 2.2, radius * 2.2);

                    for (let stripe = 0; stripe < 14; ++stripe) {
                        context.beginPath();
                        for (let point = 0; point <= 34; ++point) {
                            const offset = point / 34 * 2 - 1;
                            const px = x + offset * radius;
                            const py = y + (stripe / 13 - 0.5) * radius * 2 + Math.sin(offset * (3 + form * 2) + stripe * 0.25) * radius * 0.22;
                            if (point)
                                context.lineTo(px, py);
                            else
                                context.moveTo(px, py);
                        }
                        context.strokeStyle = rgba(stripe % 3 === 0 ? secondary : Qt.lighter(primary, 1.4), 0.1 + (stripe % 3) * 0.035);
                        context.lineWidth = root.style === "liquid" ? 4.5 : 2.4;
                        context.stroke();
                    }
                    const shade = context.createRadialGradient(x - radius * 0.3, y - radius * 0.4, radius * 0.1, x, y, radius);
                    shade.addColorStop(0, "#00ffffff");
                    shade.addColorStop(0.6, "#00081120");
                    shade.addColorStop(1, "#c0060b18");
                    context.fillStyle = shade;
                    context.fillRect(x - radius * 1.1, y - radius * 1.1, radius * 2.2, radius * 2.2);
                    context.restore();
                }
            }

            Repeater {
                model: root.detail < 0.6 ? 1 : 2

                Rectangle {
                    required property int index
                    // Кривая затухания ударной волны: быстро нарастает и плавно гаснет.
                    readonly property real shock: 1 - Math.pow(1 - root.shockProgress, 3)

                    width: Math.min(root.width, root.height) * 0.64
                    height: width
                    anchors.centerIn: parent
                    radius: width / (2.2 + root.signature * 0.6)
                    scale: (0.64 + shock * (0.28 + index * 0.08)) / 0.64
                    rotation: root.signature * 100 + index * 45 + shock * 12
                    color: "transparent"
                    border.color: index ? root.secondary : root.primary
                    border.width: index ? 1 : 1.5
                    opacity: root.reacting && settingsStore.beatEffects ? Math.pow(1 - shock, 1.4) * root.hitStrength * root.intensity * (index ? 0.18 : 0.32) : 0
                }
            }
        }
    }

    Loader {
        anchors.fill: parent
        active: root.presented && root.intensity > 0 && settingsStore.atmosphereScene === "bonsai"
        sourceComponent: BonsaiScene {
            primary: root.primary
            secondary: root.secondary
            signature: root.signature
            phase: root.phase
            energy: root.energy
            bass: root.bass
            pulse: root.pulse
            intensity: root.intensity
            detail: root.detail
            animated: root.reacting
        }
    }
    AtmosphereObjects {
        anchors.fill: parent
        visible: root.presented && enabledScene
        primary: root.primary
        secondary: root.secondary
        signature: root.signature
        phase: root.phase
        energy: root.energy
        bass: root.bass
        pulse: root.pulse
        intensity: root.intensity
        detail: root.detail
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: enabled
        acceptedButtons: Qt.NoButton
        enabled: root.interactive && root.motionRunning
        onPositionChanged: mouse => {
            root.pointerX = (mouse.x / width - 0.5) * 2;
            root.pointerY = (mouse.y / height - 0.5) * 2;
        }
        onExited: {
            root.pointerX = 0;
            root.pointerY = 0;
        }
        onEnabledChanged: if (!enabled) {
            root.pointerX = 0;
            root.pointerY = 0;
        }
    }
}
