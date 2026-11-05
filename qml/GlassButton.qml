// Кнопка в «стеклянном» стиле: полупрозрачная, с вариантом основной (primary).
import QtQuick
import QtQuick.Controls

Button {
    id: control
    property Theme uiTheme
    // Основной (акцентный) вариант кнопки.
    property bool primary: false

    implicitHeight: 42
    implicitWidth: Math.max(96, contentItem.implicitWidth + 32)
    leftPadding: 16
    rightPadding: 16
    topPadding: 0
    bottomPadding: 0
    font.pixelSize: 13
    font.weight: Font.Medium
    contentItem: Text {
        text: control.text
        font: control.font
        color: control.primary ? "#0b0c15" : (control.uiTheme ? control.uiTheme.text : "#f2f5fc")
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    background: Rectangle {
        radius: height / 2
        // Цвет фона: отключена / основная (нажата, наведена, обычная) / обычная (нажата, наведена, обычная).
        color: !control.enabled ? (control.uiTheme ? control.uiTheme.surface : "#121622") : control.primary ? (control.down ? "#9c8eff" : control.hovered ? (control.uiTheme ? control.uiTheme.accentBright : "#aa9eff") : (control.uiTheme ? control.uiTheme.accent : "#8b7cff")) : control.down ? (control.uiTheme ? control.uiTheme.outline : "#293146") : control.hovered ? (control.uiTheme ? control.uiTheme.hover : "#20283a") : (control.uiTheme ? control.uiTheme.surfaceHigh : "#1a2030")
        border.color: control.primary ? "transparent" : (control.uiTheme ? control.uiTheme.outline : "#293146")
        border.width: 1
    }
}
