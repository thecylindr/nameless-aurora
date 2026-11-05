// Кольцевой индикатор прогресса на Canvas с опцией неопределённого (вращающегося) состояния.
import QtQuick

Item {
    id: root

    // Прогресс от 0 до 1.
    property real value: 0
    property color color: "#b9a1ff"
    // Цвет фоновой (пустой) части кольца.
    property color trackColor: "#26ffffff"
    // Бесконечная анимация вместо конкретного значения.
    property bool indeterminate: false
    // Показывать ли процент по центру.
    property bool showLabel: true
    property real strokeWidth: 3
    // Анимация активна только когда окно видимо, не свёрнуто и движение разрешено.
    readonly property bool motionEnabled: settingsStore.motionEnabled && Application.state !== Qt.ApplicationHidden && Application.state !== Qt.ApplicationSuspended && Window.window !== null && Window.window.visibility !== Window.Hidden && Window.window.visibility !== Window.Minimized
    // Процент в диапазоне 0–100 с ограничением значения.
    readonly property int percentage: Math.round(Math.max(0, Math.min(1, value)) * 100)

    implicitWidth: 42
    implicitHeight: 42
    Accessible.role: Accessible.ProgressBar
    Accessible.name: indeterminate ? "Подключаем аудио" : percentage + "%"

    Canvas {
        id: arc
        anchors.fill: parent
        // Перерисовка при изменении размеров канваса.
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
        onPaint: {
            const context = getContext("2d");
            context.reset();
            // Радиус с учётом толщины обводки и зазора в 2 пикселя.
            const radius = (Math.min(width, height) - root.strokeWidth - 2) / 2;
            if (radius <= 0)
                return;
            context.lineWidth = root.strokeWidth;
            context.lineCap = "round";
            context.beginPath();
            // Полная окружность — фоновая дорожка.
            context.arc(width / 2, height / 2, radius, 0, Math.PI * 2);
            context.strokeStyle = root.trackColor;
            context.stroke();
            // Доля дуги: фиксированная при indeterminate, иначе пропорциональна значению.
            const sweep = root.indeterminate ? 0.24 : Math.max(0, Math.min(1, root.value));
            if (sweep <= 0)
                return;
            context.beginPath();
            // Дуга прогресса от верхней точки по часовой стрелке.
            context.arc(width / 2, height / 2, radius, -Math.PI / 2, -Math.PI / 2 + sweep * Math.PI * 2);
            context.strokeStyle = root.color;
            context.stroke();
        }
        // Вращение всей дуги в неопределённом состоянии.
        RotationAnimator on rotation {
            from: 0
            to: 360
            duration: 1000
            loops: Animation.Infinite
            running: root.visible && root.indeterminate && root.motionEnabled
        }
    }
    Text {
        anchors.centerIn: parent
        visible: root.showLabel && !root.indeterminate
        text: root.percentage
        color: root.color
        font.pixelSize: 11
        font.weight: Font.DemiBold
    }
    onValueChanged: arc.requestPaint()
    onColorChanged: arc.requestPaint()
    onTrackColorChanged: arc.requestPaint()
    onIndeterminateChanged: arc.requestPaint()
    onStrokeWidthChanged: arc.requestPaint()
}
