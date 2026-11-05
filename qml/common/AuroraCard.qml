// Карточка-контейнер с внутренним отступом, принимающая произвольный контент.
import QtQuick
import ".."

Rectangle {
    id: root
    property Theme uiTheme
    // Внутренний отступ вокруг вложенного содержимого.
    property real padding: 16
    // Контент карточки, вставляемый в контейнер с отступами.
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
