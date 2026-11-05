// Сцена «Бонсай»: рекурсивно нарисованное дерево и мерцающие светлячки, реагирующие на музыку.
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

    // Откладывает перерисовку дерева, чтобы смена палитры не перерисовывала Canvas несколько раз подряд.
    function schedulePaint() {
        if (!repaintDelay.running)
            repaintDelay.start();
    }

    onPrimaryChanged: schedulePaint()
    onSecondaryChanged: schedulePaint()
    onSignatureChanged: schedulePaint()

    Timer {
        id: repaintDelay
        interval: 100
        onTriggered: tree.requestPaint()
    }

    // Ветви и листва рисуются в одну кэшируемую текстуру, в том числе на программном рендере.
    Canvas {
        id: tree
        width: 384
        height: 384
        anchors.centerIn: parent
        renderStrategy: Canvas.Threaded
        transformOrigin: Item.Bottom
        scale: Math.min(root.width, root.height) / 384 * settingsStore.objectScale * (1 + root.bass * 0.02 + root.pulse * 0.015)
        rotation: root.animated ? Math.sin(root.phase * 0.3) * (0.5 + root.energy * 0.4) : 0
        opacity: Math.min(1, root.intensity * (0.8 + root.energy * 0.1 + root.pulse * 0.1)) * settingsStore.decorationOpacity

        onPaint: {
            const context = getContext("2d");
            context.reset();
            const primary = root.primary, secondary = root.secondary;
            const signature = root.signature;
            // Детерминированный генератор шума: одно и то же «семя» всегда даёт одну и ту же форму дерева.
            function random(seed) {
                const value = Math.sin(seed * 127.1 + signature * 23) * 43758.5453;
                return value - Math.floor(value);
            }
            function color(tint, alpha) {
                return Qt.rgba(tint.r, tint.g, tint.b, alpha);
            }

            const glow = context.createRadialGradient(192, 172, 20, 192, 172, 175);
            glow.addColorStop(0, color(primary, 0.15));
            glow.addColorStop(0.65, color(secondary, 0.05));
            glow.addColorStop(1, "transparent");
            context.fillStyle = glow;
            context.fillRect(0, 0, 384, 384);
            context.lineCap = "round";

            // Рекурсивно рисует ветви: на глубине ≤2 добавляет листья, глубже — делится на две ветки.
            function branch(x, y, length, angle, depth, seed) {
                const endX = x + Math.cos(angle) * length;
                const endY = y + Math.sin(angle) * length;
                context.beginPath();
                context.moveTo(x, y);
                context.quadraticCurveTo(x + Math.cos(angle - 0.18) * length * 0.55, y + Math.sin(angle - 0.18) * length * 0.55, endX, endY);
                context.lineWidth = Math.max(0.8, depth * depth * 0.36);
                context.strokeStyle = color(depth > 2 ? primary : secondary, depth > 2 ? 0.55 : 0.7);
                context.stroke();

                if (depth <= 2) {
                    for (let leaf = 0; leaf < 5; ++leaf) {
                        const spread = random(seed + leaf * 3) * Math.PI * 2;
                        const radius = 4 + random(seed + leaf + 9) * 7;
                        const distance = random(seed + leaf + 17) * 14;
                        context.beginPath();
                        context.arc(endX + Math.cos(spread) * distance, endY + Math.sin(spread) * distance * 0.6, radius, 0, Math.PI * 2);
                        context.fillStyle = color(leaf % 2 ? primary : secondary, 0.14 + random(seed + leaf) * 0.24);
                        context.fill();
                    }
                }
                if (depth <= 1)
                    return;
                const spread = 0.36 + random(seed + 4) * 0.32;
                branch(endX, endY, length * (0.68 + random(seed + 2) * 0.08), angle - spread, depth - 1, seed * 2 + 1);
                branch(endX, endY, length * (0.66 + random(seed + 3) * 0.1), angle + spread, depth - 1, seed * 2 + 2);
            }
            branch(192, 303, 64, -Math.PI / 2 + (signature - 0.5) * 0.3, 6, 1);

            context.beginPath();
            context.moveTo(151, 305);
            context.lineTo(233, 305);
            context.lineTo(223, 330);
            context.quadraticCurveTo(192, 339, 161, 330);
            context.closePath();
            context.fillStyle = color(primary, 0.23);
            context.fill();
            context.strokeStyle = color(secondary, 0.45);
            context.lineWidth = 1.3;
            context.stroke();
        }
    }

    // Мерцающие «светлячки», дрейфующие вокруг дерева и усиливающиеся на пульс.
    Repeater {
        model: settingsStore.objectDensity > 0 ? Math.max(2, Math.round(7 * root.detail)) : 0

        Rectangle {
            required property int index
            readonly property real drift: root.animated ? root.phase * (0.1 + index * 0.025) : 0

            width: 2 + index % 2
            height: width
            radius: width / 2
            x: root.width * (0.2 + (index * 0.618 % 1) * 0.6) + Math.sin(drift + index) * 8
            y: root.height * (0.2 + (index * 0.382 % 1) * 0.5) + Math.cos(drift + index) * 10
            color: index % 2 ? root.primary : root.secondary
            opacity: root.intensity * settingsStore.decorationOpacity * (0.2 + root.pulse * 0.15 + Math.sin(drift * 2 + index) * 0.08)
        }
    }
}
