import QtQuick
import QtQuick.Controls
import ".."

Button {
    id: control
    property Theme uiTheme
    property string iconName: ""
    property bool primary: false
    property bool danger: false
    property bool iconOnly: false

    implicitHeight: uiTheme ? uiTheme.touchHeight : 48
    implicitWidth: iconOnly ? implicitHeight : Math.max(96, contentItem.implicitWidth + leftPadding + rightPadding)
    leftPadding: iconOnly ? 0 : 16
    rightPadding: iconOnly ? 0 : 16
    topPadding: 0
    bottomPadding: 0
    focusPolicy: Qt.StrongFocus
    hoverEnabled: true
    font.pixelSize: uiTheme ? uiTheme.bodySize : 13
    font.weight: Font.Medium
    Accessible.name: text.length > 0 ? text : iconName

    readonly property color foreground: !enabled ? (uiTheme ? uiTheme.subtext : "#9aa4c8")
                                        : primary || danger ? "#ffffff" : (uiTheme ? uiTheme.text : "#eaf2ff")
    readonly property color baseColor: danger ? (uiTheme ? uiTheme.danger : "#ff455a")
                                      : primary ? (uiTheme ? uiTheme.accent : "#0a84ff")
                                      : (uiTheme ? uiTheme.surfaceHigh : "#1b2040")

    contentItem: Item {
        implicitWidth: row.implicitWidth
        implicitHeight: row.implicitHeight
        clip: true

        Row {
            id: row
            anchors.centerIn: parent
            spacing: control.iconOnly || control.text.length === 0 || control.iconName.length === 0 ? 0 : 8

            AuroraIcon {
                visible: control.iconName.length > 0
                uiTheme: control.uiTheme
                name: control.iconName
                color: control.foreground
                size: 18
                anchors.verticalCenter: parent.verticalCenter
            }
            Text {
                visible: !control.iconOnly && control.text.length > 0
                text: control.text
                color: control.foreground
                font: control.font
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    background: Rectangle {
        radius: height / 2
        color: !control.enabled ? (control.uiTheme ? control.uiTheme.surface : "#121626")
               : control.down ? Qt.darker(control.baseColor, 1.18)
               : control.hovered && !control.primary && !control.danger ? (control.uiTheme ? control.uiTheme.hover : "#1b2040")
               : control.hovered ? Qt.lighter(control.baseColor, 1.12)
               : control.baseColor
        border.width: control.activeFocus ? 2 : (control.primary || control.danger ? 0 : 1)
        border.color: control.activeFocus ? (control.uiTheme ? control.uiTheme.accentBright : "#5e5ce6")
                      : (control.uiTheme ? control.uiTheme.line : "#1effffff")
        Behavior on color { ColorAnimation { duration: 120 } }
    }
}
