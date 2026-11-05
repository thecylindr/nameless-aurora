// Закрашивает скруглённые углы карточки цветом фона, поверх которого рисуется анимированная атмосфера.
import QtQuick

Canvas {
    id: root
    property real radius: 22
    property color color: "#0d0f17"
    onWidthChanged: requestPaint()
    onHeightChanged: requestPaint()
    onRadiusChanged: requestPaint()
    onColorChanged: requestPaint()
    // Рисует четыре «вырезки» в углах, имитируя рамку со скруглёнными углами поверх фона.
    onPaint: {
        const c = getContext("2d");
        c.reset();
        c.fillStyle = root.color;
        const r = Math.min(root.radius, width / 2, height / 2);
        // Каждый угол задаётся смещением и поворотом, чтобы нарисовать одинаковую дугу.
        const corners = [[0, 0, 0], [width, 0, Math.PI / 2], [width, height, Math.PI], [0, height, Math.PI * 1.5]];
        for (const corner of corners) {
            c.save();
            c.translate(corner[0], corner[1]);
            c.rotate(corner[2]);
            c.beginPath();
            c.moveTo(0, 0);
            c.lineTo(r, 0);
            c.arc(r, r, r, -Math.PI / 2, -Math.PI, true);
            c.lineTo(0, 0);
            c.closePath();
            c.fill();
            c.restore();
        }
    }
}
