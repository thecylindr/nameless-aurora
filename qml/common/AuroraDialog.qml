import QtQuick
import QtQuick.Controls
import ".."

Dialog {
    id: dialog
    property Theme uiTheme
    property int maxDialogWidth: 560
    property int maxDialogHeight: 720
    property int mobileMargin: 16
    default property alias content: body.data

    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    width: Math.min(maxDialogWidth, Math.max(0, (parent ? parent.width : maxDialogWidth) - 2 * mobileMargin))
    height: Math.min(maxDialogHeight, implicitHeight, Math.max(0, (parent ? parent.height : maxDialogHeight) - 2 * mobileMargin))
    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? (parent.height - height) / 2 : 0
    leftPadding: 24
    rightPadding: 24
    topPadding: 20
    bottomPadding: 20

    contentItem: Item {
        id: body
        implicitWidth: childrenRect.width
        implicitHeight: childrenRect.height
    }

    header: Item {
        implicitHeight: dialog.title.length > 0 ? titleText.implicitHeight : 0

        Text {
            id: titleText
            width: parent.width
            visible: dialog.title.length > 0
            text: dialog.title
            color: dialog.uiTheme ? dialog.uiTheme.text : "#eaf2ff"
            font.pixelSize: dialog.uiTheme ? dialog.uiTheme.titleSize : 20
            font.weight: Font.DemiBold
            wrapMode: Text.WordWrap
        }
    }

    background: Rectangle {
        radius: dialog.uiTheme ? dialog.uiTheme.radiusLarge : 24
        color: dialog.uiTheme ? dialog.uiTheme.surface : "#121626"
        border.width: 1
        border.color: dialog.uiTheme ? dialog.uiTheme.line : "#1effffff"
    }

    Overlay.modal: Rectangle { color: "#b305070f" }
    Overlay.modeless: Rectangle { color: "#6605070f" }
}
