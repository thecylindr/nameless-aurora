// Слайдер с кастомной дорожкой и круглым ползунком, поддерживает обе ориентации.
import QtQuick
import QtQuick.Controls
import ".."

Slider {
    id: control
    property Theme uiTheme
    // Признак перетаскивания (для внешних проверок).
    readonly property bool dragged: pressed

    implicitWidth: orientation === Qt.Horizontal ? 180 : 32
    implicitHeight: orientation === Qt.Horizontal ? 32 : 180
    padding: 0
    focusPolicy: Qt.StrongFocus
    // Значение обновляется прямо во время перетаскивания.
    live: true

    background: Rectangle {
        id: track
        // Позиция дорожки зависит от ориентации: по горизонтали растянута, по вертикали узкая.
        x: control.leftPadding + (control.orientation === Qt.Horizontal ? 0 : (control.availableWidth - width) / 2)
        y: control.topPadding + (control.orientation === Qt.Horizontal ? (control.availableHeight - height) / 2 : 0)
        width: control.orientation === Qt.Horizontal ? control.availableWidth : 6
        height: control.orientation === Qt.Horizontal ? 6 : control.availableHeight
        radius: 3
        color: control.uiTheme ? control.uiTheme.surfaceHigh : "#1b2040"
        opacity: control.enabled ? 1 : 0.45

        Rectangle {
            // Заполненная часть дорожки до позиции ползунка (для вертикальной — от низа).
            width: control.orientation === Qt.Horizontal ? control.visualPosition * parent.width : parent.width
            height: control.orientation === Qt.Horizontal ? parent.height : (1 - control.visualPosition) * parent.height
            anchors.bottom: control.orientation === Qt.Vertical ? parent.bottom : undefined
            radius: parent.radius
            color: control.uiTheme ? control.uiTheme.accent : "#0a84ff"
        }
    }

    handle: Rectangle {
        // Позиция ползунка вдоль дорожки в зависимости от ориентации.
        x: control.leftPadding + (control.orientation === Qt.Horizontal ? control.visualPosition * (control.availableWidth - width) : (control.availableWidth - width) / 2)
        y: control.topPadding + (control.orientation === Qt.Horizontal ? (control.availableHeight - height) / 2 : control.visualPosition * (control.availableHeight - height))
        // Ползунок чуть увеличивается при наведении/нажатии.
        width: control.hovered || control.pressed ? 20 : 18
        height: width
        radius: width / 2
        color: control.pressed ? (control.uiTheme ? control.uiTheme.accentBright : "#5e5ce6") : (control.uiTheme ? control.uiTheme.text : "#eaf2ff")
        border.width: control.activeFocus ? 3 : 0
        border.color: control.uiTheme ? control.uiTheme.accent : "#0a84ff"
        opacity: control.enabled ? 1 : 0.45
        Behavior on width {
            enabled: control.visible && (!control.uiTheme || control.uiTheme.motionEnabled)
            NumberAnimation {
                duration: 100
            }
        }
        Behavior on color {
            enabled: control.visible && (!control.uiTheme || control.uiTheme.motionEnabled)
            ColorAnimation {
                duration: 100
            }
        }
    }
}
