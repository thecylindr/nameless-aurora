// Панель эквалайзера: 10 полос, пресеты, график кривой и предусиление.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../common" as Common

ColumnLayout {
    id: root
    property var eq: player.equalizer
    spacing: 16
    Theme {
        id: theme
    }
    RowLayout {
        Layout.fillWidth: true
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            Text {
                text: "Твой почерк звука"
                color: theme.text
                font.pixelSize: 22
                font.weight: Font.DemiBold
            }
            Text {
                Layout.fillWidth: true
                text: "10 полос · изменения слышны во время воспроизведения"
                color: theme.subtext
                font.pixelSize: 12
                wrapMode: Text.WordWrap
            }
        }
        Common.AuroraSwitch {
            uiTheme: theme
            implicitWidth: 50
            checked: root.eq.enabled
            onToggled: root.eq.enabled = checked
            Accessible.name: "Включить эквалайзер"
        }
    }
    Flow {
        Layout.fillWidth: true
        spacing: 7
        Repeater {
            model: root.eq.presets
            Common.AuroraButton {
                required property string modelData
                uiTheme: theme
                text: modelData
                primary: root.eq.preset === modelData
                implicitHeight: 34
                onClicked: root.eq.selectPreset(modelData)
            }
        }
    }
    Rectangle {
        Layout.fillWidth: true
        implicitHeight: 92
        radius: 16
        color: theme.surfaceHigh
        Canvas {
            id: curve
            anchors.fill: parent
            anchors.margins: 16
            onWidthChanged: requestPaint()
            // Рисует кривую усиления: нулевая линия по центру, точки полос на высоте, пропорциональной значению.
            onPaint: {
                const c = getContext("2d");
                c.clearRect(0, 0, width, height);
                c.strokeStyle = theme.line;
                c.lineWidth = 1;
                c.beginPath();
                c.moveTo(0, height / 2);
                c.lineTo(width, height / 2);
                c.stroke();
                const values = root.eq.gains;
                c.beginPath();
                for (let i = 0; i < values.length; i++) {
                    const x = i * width / 9, y = height / 2 - values[i] * height / 28;
                    i ? c.lineTo(x, y) : c.moveTo(x, y);
                }
                c.strokeStyle = theme.accent;
                c.lineWidth = 2;
                c.stroke();
                for (let i = 0; i < values.length; i++) {
                    c.beginPath();
                    c.arc(i * width / 9, height / 2 - values[i] * height / 28, 3, 0, Math.PI * 2);
                    c.fillStyle = theme.accent;
                    c.fill();
                }
            }
            Connections {
                target: root.eq
                function onChanged() {
                    curve.requestPaint();
                }
            }
        }
    }
    Flickable {
        id: frequencies
        Layout.fillWidth: true
        implicitHeight: 224
        contentWidth: Math.max(width, 620)
        contentHeight: height
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.horizontal: Common.AuroraScrollBar {
            uiTheme: theme
        }
        Row {
            width: frequencies.contentWidth
            height: 236
            Repeater {
                model: 10
                Column {
                    required property int index
                    width: frequencies.contentWidth / 10
                    height: 236
                    spacing: 6
                    Text {
                        width: parent.width
                        text: (root.eq.gains[parent.index] > 0 ? "+" : "") + Number(root.eq.gains[parent.index]).toFixed(1)
                        color: theme.accent
                        font.pixelSize: 11
                        horizontalAlignment: Text.AlignHCenter
                    }
                    Common.AuroraSlider {
                        uiTheme: theme
                        anchors.horizontalCenter: parent.horizontalCenter
                        orientation: Qt.Vertical
                        height: 170
                        from: -12
                        to: 12
                        stepSize: .5
                        value: root.eq.gains[parent.index]
                        onMoved: root.eq.setBand(parent.index, value)
                        opacity: root.eq.enabled ? 1 : .55
                        Accessible.name: "Полоса " + root.eq.frequencies[parent.index] + " Гц"
                    }
                    Text {
                        width: parent.width
                        // Подписи частотных полос в герцах.
                        text: ["31", "63", "125", "250", "500", "1k", "2k", "4k", "8k", "16k"][parent.index]
                        color: theme.subtext
                        font.pixelSize: 11
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }
        }
    }
    RowLayout {
        Layout.fillWidth: true
        spacing: 12
        Text {
            text: "Предусиление"
            color: theme.subtext
            font.pixelSize: 12
        }
        Common.AuroraSlider {
            uiTheme: theme
            Layout.fillWidth: true
            from: -18
            to: 6
            stepSize: .5
            value: root.eq.preamp
            onMoved: root.eq.preamp = value
        }
        Text {
            Layout.preferredWidth: 54
            text: (root.eq.preamp > 0 ? "+" : "") + root.eq.preamp.toFixed(1) + " dB"
            color: theme.text
            font.pixelSize: 11
        }
        Common.AuroraButton {
            uiTheme: theme
            iconName: "refresh"
            iconOnly: true
            text: "Сбросить частоты"
            onClicked: root.eq.reset()
        }
    }
    Common.AuroraSwitch {
        uiTheme: theme
        Layout.fillWidth: true
        text: "Компенсация усиления"
        subtext: "Сохраняет запас громкости при подъёме частот"
        checked: root.eq.autoHeadroom
        onToggled: root.eq.autoHeadroom = checked
    }
}
