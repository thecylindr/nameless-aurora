// Полоса прокрутки с тонким ползунком и подсветкой при наведении/нажатии.
import QtQuick
import QtQuick.Controls
import ".."

ScrollBar {
    id: control
    property Theme uiTheme

    // Для вертикальной ориентации — узкая и высокая, для горизонтальной — наоборот.
    implicitWidth: orientation === Qt.Vertical ? 10 : 100
    implicitHeight: orientation === Qt.Vertical ? 100 : 10
    padding: 2
    // Ползунок не сжимается меньше 8% дорожки.
    minimumSize: 0.08
    interactive: true
    focusPolicy: Qt.StrongFocus
    hoverEnabled: true

    contentItem: Rectangle {
        implicitWidth: 6
        implicitHeight: 6
        radius: 3
        // Ползунок ярче при нажатии/наведении/фокусе, иначе — приглушённый.
        color: control.pressed ? (control.uiTheme ? control.uiTheme.accent : "#0a84ff") : control.hovered || control.activeFocus ? (control.uiTheme ? control.uiTheme.subtext : "#9aa4c8") : (control.uiTheme ? control.uiTheme.muted : "#9aa4c8")
        // Скрываем ползунок, когда весь контент виден (size >= 1), и приглушаем в покое.
        opacity: control.size >= 1 ? 0 : control.active || control.hovered ? 1 : 0.5
    }

    background: Rectangle {
        radius: width / 2
        color: control.uiTheme ? control.uiTheme.surfaceHigh : "#1b2040"
        // Дорожка проявляется только при наведении или фокусе.
        opacity: control.hovered || control.activeFocus ? 0.55 : 0
    }
}
