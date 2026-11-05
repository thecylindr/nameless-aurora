// Секция с заголовком, описанием и произвольным вложенным содержимым.
import QtQuick
import ".."

Item {
    id: root
    property Theme uiTheme
    property string title: ""
    property string description: ""
    // Вложенный контент секции.
    default property alias content: body.data

    implicitWidth: 320
    implicitHeight: column.implicitHeight

    Column {
        id: column
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: root.uiTheme ? root.uiTheme.spacingSmall : 8

        Text {
            visible: root.title.length > 0
            width: parent.width
            text: root.title
            color: root.uiTheme ? root.uiTheme.text : "#eaf2ff"
            font.pixelSize: root.uiTheme ? root.uiTheme.titleSize : 20
            font.weight: Font.DemiBold
            wrapMode: Text.WordWrap
        }
        Text {
            visible: root.description.length > 0
            width: parent.width
            text: root.description
            color: root.uiTheme ? root.uiTheme.subtext : "#9aa4c8"
            font.pixelSize: root.uiTheme ? root.uiTheme.bodySize : 13
            wrapMode: Text.WordWrap
        }
        Item {
            id: body
            width: parent.width
            implicitHeight: childrenRect.height
            height: childrenRect.height
        }
    }
}
