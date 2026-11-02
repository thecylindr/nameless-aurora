import QtQuick
import QtQuick.Controls
import ".."

ProgressBar {
    id: control
    property Theme uiTheme

    implicitWidth: 180
    implicitHeight: 12
    padding: 0
    opacity: enabled ? 1 : 0.45

    background: Rectangle {
        id: track
        x: control.leftPadding
        y: control.topPadding + (control.availableHeight - height) / 2
        width: control.availableWidth
        height: 6
        radius: 3
        color: control.uiTheme ? control.uiTheme.surfaceHigh : "#1b2040"
    }

    contentItem: Item {
        implicitHeight: 6
        clip: true

        Rectangle {
            visible: !control.indeterminate
            width: control.visualPosition * parent.width
            height: parent.height
            radius: height / 2
            color: control.uiTheme ? control.uiTheme.accent : "#0a84ff"
            Behavior on width { NumberAnimation { duration: 120 } }
        }

        Rectangle {
            id: movingIndicator
            visible: control.indeterminate
            width: Math.max(48, parent.width * 0.3)
            height: parent.height
            radius: height / 2
            color: control.uiTheme ? control.uiTheme.accent : "#0a84ff"

            SequentialAnimation on x {
                running: control.indeterminate && control.visible
                loops: Animation.Infinite
                NumberAnimation {
                    from: -movingIndicator.width
                    to: movingIndicator.parent.width
                    duration: 1100
                    easing.type: Easing.InOutCubic
                }
            }
        }
    }
}
