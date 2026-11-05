import QtQuick

Item {
    id: root

    property int side: -1
    property color accent: "#b8ce87"
    property real bend: 0
    property real follow: 0
    property real tip: 0
    property real tremor: 0
    property real clock: 0

    readonly property real segmentHeight: height / 3
    readonly property real vibration: Math.sin(clock * 48 + side * 1.8) * tremor

    component Wood: Canvas {
        id: wood

        property int side: -1
        property int section: 0
        property color accent: "#b8ce87"

        width: 128
        height: 256
        renderStrategy: Canvas.Threaded
        transform: Scale {
            xScale: wood.parent.width / wood.width
            yScale: wood.parent.height / wood.height
        }
        onAccentChanged: if (!paintDelay.running) paintDelay.start()
        Timer {
            id: paintDelay
            interval: 100
            onTriggered: wood.requestPaint()
        }

        onPaint: {
            const context = getContext("2d");
            context.reset();
            const middle = 64;
            const curve = side * (section % 2 ? -21 : 24);
            const thickness = 19 - section * 3.5;
            const endThickness = section === 2 ? 0.6 : thickness * 0.73;
            const inner = -side;
            context.lineCap = "round";
            context.lineJoin = "round";

            // Side shoots curl over the frame; their tips leave the centre of the cover clear.
            for (let branch = 0; branch < 3; ++branch) {
                const y = 48 + branch * 69;
                const reach = (branch % 2 ? -inner : inner) * (28 + section * 5);
                context.beginPath();
                context.moveTo(middle, y);
                context.bezierCurveTo(middle + reach * 0.7, y + 12,
                                      middle + reach, y + 44, middle + reach * 1.2, y + 65);
                context.strokeStyle = "#633f29";
                context.lineWidth = Math.max(2, thickness * 0.48 - branch * 0.5);
                context.stroke();
                context.strokeStyle = "#b68b57";
                context.lineWidth = 1.2;
                context.stroke();
            }

            context.beginPath();
            context.moveTo(middle + side * 17, 0);
            context.bezierCurveTo(middle - side * 26, 80,
                                  middle + side * 30, 170, middle - side * 7, 256);
            context.strokeStyle = "#4b3525";
            context.lineWidth = thickness * 0.8;
            context.stroke();
            context.strokeStyle = "#aa7c48";
            context.lineWidth = thickness * 0.4;
            context.stroke();

            const bark = context.createLinearGradient(43, 0, 88, 0);
            bark.addColorStop(0, "#39291f");
            bark.addColorStop(0.34, "#94623b");
            bark.addColorStop(0.55, "#d0a66d");
            bark.addColorStop(0.72, "#795031");
            bark.addColorStop(1, "#35271f");
            context.beginPath();
            context.moveTo(middle - thickness, 0);
            context.bezierCurveTo(middle + curve - thickness, 72,
                                  middle - curve - thickness * 0.8, 174, middle - endThickness, 256);
            context.lineTo(middle + endThickness, 256);
            context.bezierCurveTo(middle - curve + thickness * 0.8, 174,
                                  middle + curve + thickness, 72, middle + thickness, 0);
            context.closePath();
            context.fillStyle = bark;
            context.fill();

            for (let grain = -1; grain <= 1; ++grain) {
                context.beginPath();
                context.moveTo(middle + grain * thickness * 0.5, 0);
                context.bezierCurveTo(middle + curve + grain * 4, 75,
                                      middle - curve + grain * 3, 170, middle + grain * thickness * 0.35, 256);
                context.strokeStyle = grain ? "#73492f" : "#e0bd80";
                context.lineWidth = grain ? 1.1 : 1.4;
                context.stroke();
            }
            // A restrained vein of colour connects the wood to the current album palette.
            context.beginPath();
            context.moveTo(middle + inner * thickness * 0.7, 8);
            context.bezierCurveTo(middle + curve + inner * 4, 82,
                                  middle - curve + inner * 4, 174, middle + inner * thickness * 0.4, 250);
            context.strokeStyle = Qt.rgba(accent.r, accent.g, accent.b, 0.55);
            context.lineWidth = 1.3;
            context.stroke();
        }
    }

    Item {
        id: upper
        width: root.width
        height: root.segmentHeight
        transformOrigin: Item.Top
        rotation: root.side * root.bend * 0.6 + root.vibration * 0.35
        Wood { side: root.side; section: 0; accent: root.accent }

        Item {
            id: middle
            y: upper.height - 1
            width: root.width
            height: root.segmentHeight + 1
            transformOrigin: Item.Top
            rotation: root.side * (root.follow - root.bend) * 1.3 - root.vibration * 0.7
            Wood { side: root.side; section: 1; accent: root.accent }

            Item {
                id: end
                y: middle.height - 1
                width: root.width
                height: root.segmentHeight + 1
                transformOrigin: Item.Top
                rotation: root.side * (root.tip - root.follow) * 1.7 + root.vibration
                Wood { side: root.side; section: 2; accent: root.accent }
            }
        }
    }
}
