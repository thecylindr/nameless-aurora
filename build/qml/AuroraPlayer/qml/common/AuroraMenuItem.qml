// Пункт меню со стрелкой для подменю и подсветкой выбранного/наведённого элемента.
import QtQuick
import QtQuick.Controls
import ".."

MenuItem {
    id: control
    required property Theme uiTheme
    implicitHeight: 40
    leftPadding: 12
    rightPadding: 28
    contentItem: Text {
        text: control.text
        font.pixelSize: 12
        color: control.enabled ? control.uiTheme.text : control.uiTheme.textMuted
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    background: Rectangle {
        radius: 10
        color: control.highlighted || control.hovered ? control.uiTheme.selected : "transparent"
    }
    arrow: AuroraIcon {
        x: control.width - width - 10
        y: (control.height - height) / 2
        name: "chevron"
        size: 15
        color: control.uiTheme.subtext
        // Стрелка видна только если у пункта есть подменю.
        visible: control.subMenu !== null
    }
}
