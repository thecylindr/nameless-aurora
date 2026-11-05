// Выпадающий список (ComboBox) в стиле Aurora с кастомным индикатором и попапом.
import QtQuick
import QtQuick.Controls
import ".."

ComboBox {
    id: control
    property Theme uiTheme
    implicitHeight: 44
    implicitWidth: 180
    leftPadding: 14
    rightPadding: 32
    font.pixelSize: 12
    contentItem: Text {
        text: control.displayText
        color: control.uiTheme ? control.uiTheme.text : "#f3f1f8"
        font: control.font
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    indicator: AuroraIcon {
        // Стрелка выпадающего списка, прижатая к правому краю.
        x: control.width - width - 12
        y: (control.height - height) / 2
        name: "chevron"
        rotation: 90
        size: 12
        color: control.uiTheme ? control.uiTheme.subtext : "#9797a7"
    }
    background: Rectangle {
        radius: 10
        color: control.uiTheme ? control.uiTheme.surface : "#15171d"
        border.color: control.activeFocus ? (control.uiTheme ? control.uiTheme.accent : "#b9a1ff") : "#18ffffff"
    }
    delegate: ItemDelegate {
        required property int index
        width: ListView.view.width
        implicitHeight: 44
        highlighted: control.highlightedIndex === index
        contentItem: Text {
            // Берём значение из роли textRole модели либо саму строку.
            text: control.textRole ? control.model[index][control.textRole] : control.model[index]
            color: control.uiTheme ? control.uiTheme.text : "#f3f1f8"
            font: control.font
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
        background: Rectangle {
            radius: 8
            color: parent.highlighted ? (control.uiTheme ? control.uiTheme.selected : "#26b9a1ff") : "transparent"
        }
    }
    popup: Popup {
        // Смещение попапа сразу под контролом.
        y: control.height + 5
        width: control.width
        // Ограничиваем высоту попапа, чтобы не растягивался за пределы экрана.
        implicitHeight: Math.min(contentItem.implicitHeight + 12, 320)
        padding: 6
        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            // Модель подключаем только когда попап открыт.
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex
            ScrollIndicator.vertical: ScrollIndicator {}
        }
        background: Rectangle {
            radius: 12
            color: control.uiTheme ? control.uiTheme.surfaceHigh : "#20232b"
            border.color: "#22ffffff"
        }
    }
}
