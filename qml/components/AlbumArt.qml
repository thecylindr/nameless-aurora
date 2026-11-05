// Обложка альбома: показывает реальное изображение, а при его отсутствии — плейсхолдер, сгенерированный из «семени».
import QtQuick
import QtQuick.Effects

Item {
    id: root
    clip: true
    property string path: ""
    property string seed: "Aurora"
    property real cornerRadius: 12
    readonly property bool hasImage: cover.status === Image.Ready
    // Преобразует строку-«семя» в стабильное число: один и тот же текст всегда даёт один и тот же цвет и рисунок.
    readonly property int seedNumber: {
        let n = 0;
        for (let i = 0; i < seed.length; ++i)
            n = (n * 31 + seed.charCodeAt(i)) & 0xffff;
        return n;
    }
    // Основной цвет плейсхолдера: оттенок берётся из хеша «семени».
    readonly property color tint: Qt.hsva((seedNumber % 360) / 360, .52, .78, 1)
    // Скругление углов через маску; включается только при аппаратном ускорении графики.
    layer.enabled: GraphicsInfo.api !== GraphicsInfo.Software
    layer.effect: MultiEffect {
        maskEnabled: true
        maskSource: mask
    }
    // Невидимый прямоугольник, который через layer.effect выше обрезает обложку по скруглённым углам.
    Rectangle {
        id: mask
        anchors.fill: parent
        radius: root.cornerRadius
        color: "white"
        layer.enabled: true
        visible: false
    }
    Rectangle {
        anchors.fill: parent
        radius: root.cornerRadius
        visible: !root.hasImage
        // Вертикальный градиент от светлого к тёмному имитирует глубину и объём обложки.
        gradient: Gradient {
            GradientStop {
                position: 0
                color: Qt.lighter(root.tint, 1.12)
            }
            GradientStop {
                position: .55
                color: Qt.darker(root.tint, 1.7)
            }
            GradientStop {
                position: 1
                color: Qt.darker(root.tint, 3.4)
            }
        }
        Item {
            width: Math.min(parent.width, parent.height) * .88
            height: width
            anchors.centerIn: parent
            // Пять концентрических колец образуют рисунок виниловой пластинки.
            Repeater {
                model: 5
                Rectangle {
                    required property int index
                    width: parent.width * (1 - index * .145)
                    height: width
                    radius: width / 2
                    anchors.centerIn: parent
                    color: "transparent"
                    border.width: Math.max(1, root.width * .004)
                    border.color: Qt.rgba(1, 1, 1, .08 + index * .018)
                }
            }
            // Центральная «этикетка» с первой буквой названия.
            Rectangle {
                width: parent.width * .34
                height: width
                radius: width / 2
                anchors.centerIn: parent
                color: Qt.rgba(root.tint.r, root.tint.g, root.tint.b, .32)
                border.color: "#30ffffff"
                border.width: Math.max(1, root.width * .003)
                Text {
                    anchors.fill: parent
                    text: root.seed.trim().slice(0, 1).toUpperCase() || "♪"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    color: "#efffffff"
                    font.pixelSize: root.width * .18
                    font.weight: Font.Medium
                }
            }
        }
        Rectangle {
            anchors.fill: parent
            radius: root.cornerRadius
            color: "transparent"
            border.color: "#18ffffff"
            border.width: 1
        }
    }
    Image {
        id: cover
        anchors.fill: parent
        // Локальный путь кодируется посимвольно, чтобы корректно открывались имена с пробелами и кириллицей.
        source: root.path.startsWith("https://") ? root.path : root.path.length ? "file://" + root.path.split('/').map(encodeURIComponent).join('/') : ""
        fillMode: Image.PreserveAspectCrop
        sourceSize: Qt.size(640, 640)
        asynchronous: true
        visible: root.hasImage
    }
}
