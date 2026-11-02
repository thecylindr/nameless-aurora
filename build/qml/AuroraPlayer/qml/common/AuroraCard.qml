import QtQuick
import ".."

Rectangle {
    id: root
    property Theme uiTheme
    property real padding: 16
    default property alias content: container.data

    implicitWidth: container.childrenRect.width + 2 * padding
    implicitHeight: container.childrenRect.height + 2 * padding
    color: uiTheme ? uiTheme.surface : "#121626"
    border.color: uiTheme ? uiTheme.line : "#1effffff"
    border.width: 1
    radius: uiTheme ? uiTheme.radius : 16

    Item {
        id: container
        anchors.fill: parent
        anchors.margins: root.padding
    }
}
