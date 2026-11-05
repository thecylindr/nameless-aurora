// Модальное диалоговое окно с заголовком, прокручиваемым телом и кнопками действий.
import QtQuick
import QtQuick.Controls
import ".."

Dialog {
    id: dialog
    property Theme uiTheme
    property string acceptLabel: "Готово"
    property int maxDialogWidth: 560
    property int maxDialogHeight: 720
    // Отступ от краёв экрана на мобильных устройствах.
    property int mobileMargin: 16
    default property alias content: body.data

    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    // Ширина и высота ограничены максимумом и свободным местом родителя (с учётом отступа).
    width: Math.min(maxDialogWidth, Math.max(0, (parent ? parent.width : maxDialogWidth) - 2 * mobileMargin))
    height: Math.min(maxDialogHeight, implicitHeight, Math.max(0, (parent ? parent.height : maxDialogHeight) - 2 * mobileMargin))
    // Центрируем диалог относительно родителя.
    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? (parent.height - height) / 2 : 0
    leftPadding: 24
    rightPadding: 24
    topPadding: 20
    bottomPadding: 20

    contentItem: ScrollView {
        id: scroll
        implicitWidth: 0
        implicitHeight: body.implicitHeight
        contentWidth: availableWidth
        contentHeight: body.implicitHeight
        clip: true
        ScrollBar.vertical: AuroraScrollBar {
            uiTheme: dialog.uiTheme
            x: scroll.width - width
            height: scroll.height
        }
        Item {
            id: body
            width: scroll.availableWidth
            height: implicitHeight
            implicitHeight: childrenRect.height
        }
    }

    header: Item {
        implicitHeight: dialog.title.length ? titleText.implicitHeight + 40 : 0
        Text {
            id: titleText
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 24
            anchors.rightMargin: 24
            text: dialog.title
            color: dialog.uiTheme ? dialog.uiTheme.text : "#eaf2ff"
            font.pixelSize: dialog.uiTheme ? dialog.uiTheme.titleSize : 20
            font.weight: Font.DemiBold
            wrapMode: Text.WordWrap
        }
    }
    footer: DialogButtonBox {
        id: actions
        standardButtons: dialog.standardButtons
        visible: standardButtons !== Dialog.NoButton
        alignment: Qt.AlignRight
        leftPadding: 20
        rightPadding: 20
        topPadding: 4
        bottomPadding: 20
        spacing: 10
        background: Item {}
        delegate: AuroraButton {
            uiTheme: dialog.uiTheme
            // Принимающая/утверждающая роль рисуется как основная кнопка.
            primary: DialogButtonBox.buttonRole === DialogButtonBox.AcceptRole || DialogButtonBox.buttonRole === DialogButtonBox.YesRole
        }
        onAccepted: dialog.accept()
        onRejected: dialog.reject()
    }
    // После открытия подставляем русские подписи кнопок согласно их ролям.
    onOpened: {
        for (let i = 0; i < actions.count; i++) {
            const button = actions.itemAt(i);
            const role = button.DialogButtonBox.buttonRole;
            if (role === DialogButtonBox.AcceptRole)
                button.text = dialog.acceptLabel;
            else if (role === DialogButtonBox.YesRole)
                button.text = "Да";
            else if (role === DialogButtonBox.NoRole)
                button.text = "Нет";
            else if (role === DialogButtonBox.RejectRole)
                button.text = "Отмена";
        }
    }

    background: Rectangle {
        radius: dialog.uiTheme ? dialog.uiTheme.radiusLarge : 24
        color: dialog.uiTheme ? dialog.uiTheme.surface : "#121626"
        border.width: 1
        border.color: dialog.uiTheme ? dialog.uiTheme.line : "#1effffff"
    }

    Overlay.modal: Rectangle {
        // Полупрозрачная подложка под модальным диалогом.
        color: "#b305070f"
    }
    Overlay.modeless: Rectangle {
        color: "#6605070f"
    }
}
