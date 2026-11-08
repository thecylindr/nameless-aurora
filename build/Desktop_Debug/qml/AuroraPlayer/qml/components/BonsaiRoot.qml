import QtQuick
import QtQuick.Shapes

// One continuous, tapered tendril. Its attachment stays fixed while the tip curls.
Item {
    id: root

    property color accent: "#83dbc7"
    property real bend: 0
    property real follow: 0
    property real tip: 0
    property real clock: 0
    property real seed: 0
    property bool animated: true
    readonly property real sway: animated ? Math.sin(clock * 1.6 + seed * 5) : 0
    readonly property real curl: width * (0.18 * sway + bend * 0.055)
    readonly property real tail: width * (0.26 * Math.sin(seed * 7 + (animated ? clock * 1.1 : 0)) + tip * 0.12)

    Repeater {
        model: 3

        Shape {
            id: layer

            required property int index

            anchors.fill: parent
            preferredRendererType: Shape.CurveRenderer

            ShapePath {
                strokeColor: layer.index === 0 ? Qt.rgba(root.accent.r, root.accent.g, root.accent.b, 0.13) : layer.index === 1 ? root.accent : Qt.lighter(root.accent, 1.7)
                strokeWidth: layer.index === 0 ? 9 : layer.index === 1 ? 2.6 : 0.85
                fillColor: "transparent"
                capStyle: ShapePath.RoundCap
                joinStyle: ShapePath.RoundJoin
                startX: root.width / 2
                startY: 0

                PathCubic {
                    control1X: root.width / 2 - root.curl * 0.3
                    control1Y: root.height * 0.12
                    control2X: root.width / 2 + root.curl
                    control2Y: root.height * 0.22
                    x: root.width / 2 + root.curl * 0.4
                    y: root.height * 0.38
                }

                PathCubic {
                    control1X: root.width / 2 - root.curl * 0.9
                    control1Y: root.height * 0.52
                    control2X: root.width / 2 - root.width * 0.23 - root.follow * root.width * 0.06
                    control2Y: root.height * 0.62
                    x: root.width / 2 + root.tail * 0.4
                    y: root.height * 0.76
                }

                PathCubic {
                    control1X: root.width / 2 + root.tail
                    control1Y: root.height * 0.85
                    control2X: root.width / 2 + root.tail * 1.4
                    control2Y: root.height * 0.92
                    x: root.width / 2 + root.tail * 0.8
                    y: root.height
                }

            }

        }

    }

}
