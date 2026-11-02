import QtQuick
import ".."

Item {
    id: root
    property Theme uiTheme
    property string message: ""
    property int duration: 3000
    property int maximumWidth: 520
    property int mobileMargin: 16

    anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined
    anchors.bottom: parent ? parent.bottom : undefined
    anchors.bottomMargin: 24
    width: Math.min(maximumWidth, Math.max(0, (parent ? parent.width : maximumWidth) - 2 * mobileMargin))
    height: card.height
    z: 1000
    opacity: 0
    visible: opacity > 0
    Accessible.role: Accessible.Alert
    Accessible.name: message
    Accessible.ignored: !visible

    function show(text) {
        message = String(text)
        opacity = 1
        hideTimer.restart()
    }

    function hide() {
        hideTimer.stop()
        opacity = 0
    }

    Behavior on opacity { NumberAnimation { duration: 180 } }
    Timer {
        id: hideTimer
        interval: root.duration
        onTriggered: root.opacity = 0
    }

    Rectangle {
        id: card
        width: root.width
        height: label.implicitHeight + 24
        radius: root.uiTheme ? root.uiTheme.radius : 16
        color: root.uiTheme ? root.uiTheme.surfaceHigh : "#1b2040"
        border.width: 1
        border.color: root.uiTheme ? root.uiTheme.line : "#1effffff"

        Text {
            id: label
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 12
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            text: root.message
            color: root.uiTheme ? root.uiTheme.text : "#eaf2ff"
            font.pixelSize: root.uiTheme ? root.uiTheme.bodySize : 13
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }
    }
}
