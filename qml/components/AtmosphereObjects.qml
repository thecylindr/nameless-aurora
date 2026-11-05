// Декоративные объекты сцен (призмы, пузыри, космос, метеоры, ленты), раскладываемые детерминированно по размеру экрана.
import QtQuick
import QtQuick.Shapes

Item {
    id: root

    property color primary: "#baa5ff"
    property color secondary: "#83dbc7"
    property string scene: settingsStore.atmosphereScene
    property real density: settingsStore.objectDensity
    property real detail: 1
    property real phase: 0
    property real energy: 0
    property real bass: 0
    property real pulse: 0
    property real signature: 0.5
    property real intensity: settingsStore.atmosphereIntensity

    readonly property bool enabledScene: scene !== "off" && scene !== "orb" && scene !== "bonsai" && density > 0 && intensity > 0 && width > 0 && height > 0
    readonly property real sceneSize: Math.min(width, height)
    // Бюджет объектов масштабируется от размера сцены и уровня детализации, не перегружая слабые экраны.
    readonly property real objectBudget: Math.max(0.25, Math.min(1, sceneSize / 520) * detail)

    readonly property bool floatingScene: ["prisms", "bubbles", "cosmos"].indexOf(scene) >= 0
    readonly property int floatingCount: Math.max(1, Math.round((5 + density * 15) * objectBudget))
    readonly property int meteorCount: Math.max(2, Math.round((4 + density * 14) * objectBudget))
    readonly property int ribbonCount: Math.max(1, Math.round((3 + density * 3) * objectBudget))

    visible: enabledScene
    clip: true

    // Пути фигур остаются неизменными — двигаются только трансформации графа сцены.
    Repeater {
        model: root.visible && root.enabledScene && root.floatingScene ? root.floatingCount : 0

        delegate: Item {
            id: object

            required property int index
            // «Золотое сечение» даёт детерминированное, но равномерное распределение объектов без случайности.
            readonly property real seed: (index * 0.61803398875 + root.signature * 0.31) % 1
            readonly property bool bubble: root.scene === "bubbles" || (root.scene === "cosmos" && index % 3 === 0)
            readonly property bool spark: root.scene === "cosmos" && index % 3 === 1
            readonly property color tint: index % 2 ? root.primary : root.secondary
            readonly property real drift: root.phase * (0.14 + (index % 4) * 0.03)

            width: (spark ? 3 : bubble ? 22 + (index % 4) * 14 : 24 + (index % 5) * 9) * root.sceneSize / 520 * settingsStore.objectScale
            height: width
            x: root.width * (0.1 + ((index * 0.381966 + root.signature * 0.23) % 1) * 0.8) + Math.sin(drift + index * 2.3) * root.width * 0.05 - width / 2
            y: root.height * (0.1 + ((index * 0.732051 + root.signature * 0.41) % 1) * 0.8) + Math.cos(drift * 0.8 + index) * root.height * 0.06 - height / 2
            rotation: bubble ? 0 : root.phase * (index % 2 ? 4 : -3) + index * 47 + root.signature * 120
            scale: 1 + root.bass * 0.12 + root.pulse * (0.12 + seed * 0.18)
            opacity: Math.min(0.62, (spark ? 0.32 : bubble ? 0.15 : 0.22) * root.intensity * (0.75 + 0.25 * Math.sin(drift * 2 + index)) + root.pulse * 0.12)

            Loader {
                anchors.fill: parent
                sourceComponent: object.bubble || object.spark ? roundObject : prismObject
            }

            Component {
                id: roundObject

                Rectangle {
                    radius: width / 2
                    color: object.spark ? object.tint : Qt.rgba(object.tint.r, object.tint.g, object.tint.b, 0.13)
                    border.width: object.spark ? 0 : 1
                    border.color: object.tint

                    Rectangle {
                        visible: object.bubble
                        width: parent.width * 0.15
                        height: width
                        radius: width / 2
                        x: parent.width * 0.24
                        y: parent.height * 0.2
                        color: "#b0ffffff"
                    }
                }
            }

            Component {
                id: prismObject

                Shape {
                    preferredRendererType: Shape.CurveRenderer

                    ShapePath {
                        strokeWidth: 1.2
                        strokeColor: object.tint
                        fillColor: Qt.rgba(object.tint.r, object.tint.g, object.tint.b, 0.12)
                        startX: object.width * 0.5
                        startY: 0
                        PathLine {
                            x: object.width
                            y: object.height * 0.86
                        }
                        PathLine {
                            x: 0
                            y: object.height * 0.86
                        }
                        PathLine {
                            x: object.width * 0.5
                            y: 0
                        }
                    }
                    ShapePath {
                        strokeWidth: 0.6
                        strokeColor: object.tint
                        fillColor: "transparent"
                        startX: object.width * 0.5
                        startY: object.height * 0.57
                        PathLine {
                            x: object.width * 0.5
                            y: 0
                        }
                        PathMove {
                            x: object.width * 0.5
                            y: object.height * 0.57
                        }
                        PathLine {
                            x: object.width
                            y: object.height * 0.86
                        }
                        PathMove {
                            x: object.width * 0.5
                            y: object.height * 0.57
                        }
                        PathLine {
                            x: 0
                            y: object.height * 0.86
                        }
                    }
                }
            }
        }
    }

    Repeater {
        model: root.visible && root.enabledScene && root.scene === "cosmos" ? (root.detail < 0.6 ? 1 : 2) : 0

        delegate: Rectangle {
            required property int index

            anchors.centerIn: parent
            width: root.width * (0.72 + index * 0.18)
            height: width * 0.38
            radius: width / 2
            color: "transparent"
            border.width: 1
            border.color: index ? root.secondary : root.primary
            scale: 1 + root.bass * 0.035
            rotation: -24 + index * 50 + Math.sin(root.phase * 0.08 + root.signature * 6) * 12
            opacity: root.intensity * (0.09 + root.pulse * 0.045)
        }
    }

    Repeater {
        id: meteors
        objectName: "meteorRepeater"
        model: root.visible && root.enabledScene && root.scene === "meteors" ? root.meteorCount : 0

        delegate: Item {
            id: meteor
            objectName: "meteor" + index

            required property int index
            readonly property real seed: (index * 0.61803398875 + 0.17) % 1
            readonly property real lane: (index * 0.754877666 + 0.31) % 1
            readonly property real depth: 0.35 + seed * 0.65
            readonly property color tint: index % 2 ? root.primary : root.secondary
            // Метеор летит по диагонали экрана: направление, старт у края и длительность цикла зависят от «дорожки» и глубины.
            readonly property real angle: 32 + lane * 9
            readonly property real directionX: Math.cos(angle * Math.PI / 180)
            readonly property real directionY: Math.sin(angle * Math.PI / 180)
            readonly property real tailLength: Math.max(24, Math.min(150, root.sceneSize * (0.1 + depth * 0.15)))
            readonly property real margin: tailLength + 12
            readonly property real topWeight: root.width / (root.width + root.height / Math.tan(angle * Math.PI / 180))
            readonly property bool fromTop: lane < topWeight
            readonly property real startX: fromTop ? (lane / topWeight) * root.width : -margin
            readonly property real startY: fromTop ? -margin : ((lane - topWeight) / (1 - topWeight)) * root.height
            readonly property real distance: Math.min((root.width + margin - startX) / directionX, (root.height + margin - startY) / directionY)
            readonly property real cycleDuration: 1.8 + (1 - depth) * 2.4
            readonly property real flightFraction: 0.62 + depth * 0.2
            readonly property real travel: ((root.phase / cycleDuration + seed) % 1) / flightFraction
            readonly property real envelope: Math.min(1, travel / 0.12) * Math.max(0, Math.min(1, (1 - travel) / 0.18))

            x: startX + directionX * distance * travel
            y: startY + directionY * distance * travel
            rotation: angle
            scale: 1 + root.pulse * 0.06
            visible: travel < 1
            opacity: envelope * Math.min(0.95, root.intensity * (0.3 + depth * 0.42 + root.pulse * 0.12))

            // Голова метеора находится в начале координат; хвост вытянут по направлению полёта.
            Rectangle {
                x: -width
                y: -height / 2
                width: meteor.tailLength
                height: 1 + meteor.depth * 1.4
                radius: height / 2
                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop {
                        position: 0
                        color: "transparent"
                    }
                    GradientStop {
                        position: 0.55
                        color: Qt.rgba(meteor.tint.r, meteor.tint.g, meteor.tint.b, 0.3)
                    }
                    GradientStop {
                        position: 0.9
                        color: meteor.tint
                    }
                    GradientStop {
                        position: 1
                        color: "#f0ffffff"
                    }
                }
            }
            Rectangle {
                x: -width
                y: -height / 2
                width: meteor.tailLength * 0.75
                height: 3 + meteor.depth * 3
                radius: height / 2
                opacity: 0.13
                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop {
                        position: 0
                        color: "transparent"
                    }
                    GradientStop {
                        position: 1
                        color: meteor.tint
                    }
                }
            }
            Rectangle {
                width: 10 + meteor.depth * 6
                height: width
                x: -width / 2
                y: -height / 2
                radius: width / 2
                color: meteor.tint
                opacity: 0.08 + root.pulse * 0.04
            }
            Rectangle {
                width: 2.5 + meteor.depth * 2
                height: width
                x: -width / 2
                y: -height / 2
                radius: width / 2
                color: "#f5ffffff"
            }
        }
    }

    // Парящие ленты света — крупные кубические кривые, слегка реагирующие на бас и пульс.
    Repeater {
        model: root.visible && root.enabledScene && root.scene === "ribbons" ? root.ribbonCount : 0

        delegate: Shape {
            id: ribbon
            required property int index
            readonly property color tint: index % 2 ? root.primary : root.secondary

            width: root.width * 1.25
            height: root.height * 0.55
            anchors.centerIn: parent
            rotation: -24 + index * 17 + Math.sin(root.phase * 0.2 + index) * 9
            scale: 1 + root.bass * 0.04 + root.pulse * 0.03
            opacity: root.intensity * (0.4 + Math.sin(root.phase * 0.35 + index) * 0.12 + root.pulse * 0.12)
            preferredRendererType: Shape.CurveRenderer

            ShapePath {
                fillColor: "transparent"
                strokeColor: Qt.rgba(ribbon.tint.r, ribbon.tint.g, ribbon.tint.b, 0.13)
                strokeWidth: 22 + ribbon.index * 7
                startX: 0
                startY: ribbon.height * 0.6
                PathCubic {
                    x: ribbon.width
                    y: ribbon.height * 0.3
                    control1X: ribbon.width * 0.3
                    control1Y: -ribbon.height * 0.6
                    control2X: ribbon.width * 0.65
                    control2Y: ribbon.height * 1.6
                }
            }
            ShapePath {
                fillColor: "transparent"
                strokeColor: Qt.rgba(ribbon.tint.r, ribbon.tint.g, ribbon.tint.b, 0.65)
                strokeWidth: 1.3
                startX: 0
                startY: ribbon.height * 0.6
                PathCubic {
                    x: ribbon.width
                    y: ribbon.height * 0.3
                    control1X: ribbon.width * 0.3
                    control1Y: -ribbon.height * 0.6
                    control2X: ribbon.width * 0.65
                    control2Y: ribbon.height * 1.6
                }
            }
        }
    }
}
