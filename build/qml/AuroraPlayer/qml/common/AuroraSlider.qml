import QtQuick
import QtQuick.Controls
import ".."

Slider {
    id: control
    property Theme uiTheme
    readonly property bool dragged: pressed

    implicitWidth: orientation === Qt.Horizontal ? 180 : 32
    implicitHeight: orientation === Qt.Horizontal ? 32 : 180
    padding: 0
    focusPolicy: Qt.StrongFocus
    live: true

    background: Rectangle {
        id: track
        x: control.leftPadding + (control.orientation === Qt.Horizontal ? 0 : (control.availableWidth - width) / 2)
        y: control.topPadding + (control.orientation === Qt.Horizontal ? (control.availableHeight - height) / 2 : 0)
        width: control.orientation === Qt.Horizontal ? control.availableWidth : 6
        height: control.orientation === Qt.Horizontal ? 6 : control.availableHeight
        radius: 3
        color: control.uiTheme ? control.uiTheme.surfaceHigh : "#1b2040"
        opacity: control.enabled ? 1 : 0.45

        Rectangle {
            width: control.orientation === Qt.Horizontal ? control.visualPosition * parent.width : parent.width
            height: control.orientation === Qt.Horizontal ? parent.height : control.visualPosition * parent.height
            anchors.bottom: control.orientation === Qt.Vertical ? parent.bottom : undefined
            radius: parent.radius
            color: control.uiTheme ? control.uiTheme.accent : "#0a84ff"
        }
    }

    handle: Rectangle {
        x: control.leftPadding + (control.orientation === Qt.Horizontal
           ? control.visualPosition * (control.availableWidth - width)
           : (control.availableWidth - width) / 2)
        y: control.topPadding + (control.orientation === Qt.Horizontal
           ? (control.availableHeight - height) / 2
           : (1 - control.visualPosition) * (control.availableHeight - height))
        width: control.hovered || control.pressed ? 20 : 18
        height: width
        radius: width / 2
        color: control.pressed ? (control.uiTheme ? control.uiTheme.accentBright : "#5e5ce6")
               : (control.uiTheme ? control.uiTheme.text : "#eaf2ff")
        border.width: control.activeFocus ? 3 : 0
        border.color: control.uiTheme ? control.uiTheme.accent : "#0a84ff"
        opacity: control.enabled ? 1 : 0.45
        Behavior on width { NumberAnimation { duration: 100 } }
        Behavior on color { ColorAnimation { duration: 100 } }
    }
}
