// Фоновая «атмосфера»: световые пятна и линии, реагирующие на ритм и цвет текущего трека.
import QtQuick
import ".."

EffectSurface {
    id: root

    // Палитра и характер сцены берутся из атмосферы текущего трека или из темы.
    property color tint: player.hasTrack ? player.atmosphereColor : theme.accent
    property color secondary: player.hasTrack ? player.atmosphereSecondary : theme.accent2
    property real signature: player.hasTrack ? player.atmosphereShape : 0.5
    property real intensity: settingsStore.atmosphereScene === "off" ? 0 : settingsStore.atmosphereIntensity
    property real energy: 0
    property real bass: 0
    property real pulse: 0
    property real phase: 0
    property bool paintPending: true

    animated: player.playing
    lively: player.playing
    motionEnabled: intensity > 0 && settingsStore.atmosphereScene !== "off"
    clip: true

    Behavior on tint {
        enabled: root.motionRunning
        ColorAnimation {
            duration: 1100
        }
    }
    Behavior on secondary {
        enabled: root.motionRunning
        ColorAnimation {
            duration: 1300
        }
    }

    // Откладывает перерисовку текстуры света на короткий таймер, чтобы не дёргать Canvas на каждое изменение.
    function schedulePaint() {
        paintPending = true;
        if (presented && !repaintDelay.running)
            repaintDelay.start();
    }

    onTintChanged: schedulePaint()
    onSecondaryChanged: schedulePaint()
    onSignatureChanged: schedulePaint()
    onPresentedChanged: if (presented && paintPending)
        schedulePaint()
    onMotionRunningChanged: {
        if (!motionRunning) {
            energy = 0;
            bass = 0;
            pulse = 0;
        }
    }
    // Плавно подтягивает энергию, бас и пульс к целям из анализатора; экспоненциальное сглаживание убирает рывки.
    onTick: seconds => {
        const strength = settingsStore.reactionStrength;
        const reacting = player.playing;
        const targetEnergy = reacting ? Math.min(1.5, player.audioEnergy * strength) : 0;
        const targetBass = reacting ? Math.min(1.5, player.bassEnergy * strength) : 0;
        const targetPulse = reacting && settingsStore.beatEffects ? Math.min(1.5, player.beatPulse * strength) : 0;
        energy += (targetEnergy - energy) * (1 - Math.exp(-seconds / 0.11));
        bass += (targetBass - bass) * (1 - Math.exp(-seconds / 0.08));
        pulse += (targetPulse - pulse) * (1 - Math.exp(-seconds / 0.045));
        phase += seconds * settingsStore.effectSpeed * (0.35 + energy + pulse * 0.6);
    }

    Theme {
        id: theme
    }

    // Собирает несколько изменений палитры в одну перерисовку раз в 100 мс.
    Timer {
        id: repaintDelay
        interval: 100
        onTriggered: {
            if (root.presented) {
                lights.requestPaint();
                root.paintPending = false;
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        color: theme.surface
    }

    // Перерисовываем маленькую текстуру только при смене палитры; движение задаётся трансформациями.
    Canvas {
        id: lights
        width: 256
        height: 160
        anchors.centerIn: parent
        visible: root.intensity > 0 && settingsStore.atmosphereScene !== "off"
        // Масштаб, поворот и смещение реагируют на фазу, бас и пульс, не требуя перерисовки Canvas.
        scale: Math.max(root.width / width, root.height / height) * (1.12 + root.bass * 0.09 + root.pulse * 0.045)
        rotation: Math.sin(root.phase * 0.31) * (3 + root.energy * 4)
        transform: Translate {
            x: Math.sin(root.phase * 0.44) * root.width * (0.018 + root.energy * 0.023)
            y: Math.cos(root.phase * 0.36) * root.height * (0.012 + root.bass * 0.025)
        }
        opacity: Math.min(1, root.intensity * (0.49 + root.energy * 0.2 + root.pulse * 0.23))
        renderStrategy: Canvas.Threaded

        // Рисуем радиальные световые пятна и светящиеся полосы в маленькую текстуру 256×160.
        onPaint: {
            const context = getContext("2d");
            context.reset();
            function glowAt(x, y, radius, color) {
                const gradient = context.createRadialGradient(x, y, 0, x, y, radius);
                gradient.addColorStop(0, color);
                gradient.addColorStop(1, "transparent");
                context.fillStyle = gradient;
                context.fillRect(0, 0, 256, 160);
            }
            const tint = Qt.rgba(root.tint.r, root.tint.g, root.tint.b, 0.85);
            const second = Qt.rgba(root.secondary.r, root.secondary.g, root.secondary.b, 0.72);
            glowAt(150 + root.signature * 70, 35 + root.signature * 20, 150, tint);
            glowAt(40, 120, 135, second);
            glowAt(225, 150, 100, Qt.rgba(root.tint.r, root.tint.g, root.tint.b, 0.5));

            const line = context.createLinearGradient(0, 160, 256, 0);
            line.addColorStop(0, "transparent");
            line.addColorStop(0.45, Qt.rgba(second.r, second.g, second.b, 0.06));
            line.addColorStop(0.72, Qt.rgba(tint.r, tint.g, tint.b, 0.15));
            line.addColorStop(1, "transparent");
            context.strokeStyle = line;
            for (let stripe = 0; stripe < 5; ++stripe) {
                context.beginPath();
                context.moveTo(-20, 145 + stripe * 7);
                context.bezierCurveTo(60, 125 - stripe * 6, 80 + root.signature * 65, -15 + stripe * 9, 280, 15 + stripe * 10);
                context.lineWidth = 2.5 + stripe * 0.7;
                context.stroke();
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop {
                position: 0
                color: Qt.rgba(theme.base.r, theme.base.g, theme.base.b, 0.1)
            }
            GradientStop {
                position: 1
                color: Qt.rgba(theme.base.r, theme.base.g, theme.base.b, 0.86)
            }
        }
    }
}
