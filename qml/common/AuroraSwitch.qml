// Переключатель (Switch) с текстовой меткой, подписью и анимированным ползунком.
import QtQuick
import QtQuick.Controls
import ".."

Switch {
    id: control
    property Theme uiTheme
    // Заголовок переключателя.
    property string label: ""
    // Дополнительное описание под заголовком.
    property string subtext: ""

    implicitWidth: 240
    // Высота не меньше сенсорного минимума и содержимого.
    implicitHeight: Math.max(uiTheme ? uiTheme.touchHeight : 48, contentItem.implicitHeight + topPadding + bottomPadding)

    spacing: 12
    focusPolicy: Qt.StrongFocus
    hoverEnabled: true
    opacity: enabled ? 1 : 0.45
    // Имя для доступности — предпочтительно label, иначе text.
    Accessible.name: label.length > 0 ? label : text

    contentItem: Column {
        spacing: 3

        Text {
            width: parent.width
            text: control.label.length > 0 ? control.label : control.text
            color: control.uiTheme ? control.uiTheme.text : "#eaf2ff"
            font.pixelSize: control.uiTheme ? control.uiTheme.bodySize : 13
            font.weight: Font.Medium
            elide: Text.ElideRight
        }
        Text {
            visible: control.subtext.length > 0
            width: parent.width
            text: control.subtext
            color: control.uiTheme ? control.uiTheme.subtext : "#9aa4c8"
            font.pixelSize: control.uiTheme ? control.uiTheme.captionSize : 11
            wrapMode: Text.WordWrap
        }
    }

    indicator: Rectangle {
        // Индикатор прижимается к противоположному от текста краю (с учётом зеркалирования).
        x: control.mirrored ? control.leftPadding : control.width - control.rightPadding - width
        y: control.topPadding + (control.availableHeight - height) / 2
        implicitWidth: 46
        implicitHeight: 26
        radius: 13
        // Фон индикатора: акцентный когда включён, иначе приглушённый.
        color: control.checked ? (control.uiTheme ? control.uiTheme.accent : "#0a84ff") : (control.uiTheme ? control.uiTheme.surfaceHigh : "#1b2040")
        border.width: control.activeFocus ? 2 : 1
        border.color: control.activeFocus || control.hovered ? (control.uiTheme ? control.uiTheme.accentBright : "#5e5ce6") : (control.uiTheme ? control.uiTheme.line : "#1effffff")
        Behavior on color {
            enabled: control.visible && (!control.uiTheme || control.uiTheme.motionEnabled)
            ColorAnimation {
                duration: 140
            }
        }

        Rectangle {
            // Круглый ползунок скользит между краями; смещение зависит от состояния.
            x: control.checked ? parent.width - width - 3 : 3
            y: 3
            width: 20
            height: 20
            radius: 10
            color: "#ffffff"
            Behavior on x {
                enabled: control.visible && (!control.uiTheme || control.uiTheme.motionEnabled)
                NumberAnimation {
                    duration: 140
                    easing.type: Easing.OutCubic
                }
            }
        }
    }
}
