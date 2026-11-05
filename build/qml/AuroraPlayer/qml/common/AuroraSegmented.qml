pragma ComponentBehavior: Bound
// Сегментированный переключатель: несколько опций, из которых активна одна.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ".."

Item {
    id: root
    property Theme uiTheme
    // Модель опций: массив строк или объектов.
    property var model: []
    // Индекс выбранной опции.
    property int currentIndex: 0
    property string valueRole: "value"
    property string textRole: "text"
    // Сигнал выбора опции с её индексом и значением.
    signal optionSelected(int index, var value)

    // Минимальная ширина зависит от количества опций.
    implicitWidth: Math.max(160, repeater.count * 96)
    implicitHeight: uiTheme ? uiTheme.controlHeight : 44

    // Возвращает текст опции, поддерживая строки и объекты с разными ролями.
    function optionText(option) {
        if (option === null || option === undefined)
            return "";
        if (typeof option !== "object")
            return String(option);
        if (option[textRole] !== undefined)
            return String(option[textRole]);
        if (option.text !== undefined)
            return String(option.text);
        if (option.label !== undefined)
            return String(option.label);
        return String(option);
    }

    // Возвращает значение опции: из роли valueRole или сам объект.
    function optionValue(option) {
        if (option !== null && option !== undefined && typeof option === "object" && option[valueRole] !== undefined)
            return option[valueRole];
        return option;
    }

    Rectangle {
        anchors.fill: parent
        radius: height / 2
        color: root.uiTheme ? root.uiTheme.surface : "#121626"
        border.width: 1
        border.color: root.uiTheme ? root.uiTheme.line : "#1effffff"
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 3
        spacing: 3

        Repeater {
            id: repeater
            model: root.model

            delegate: Button {
                id: optionButton
                required property int index
                required property var modelData

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 1
                Layout.minimumWidth: 0
                implicitWidth: 0
                padding: 8
                focusPolicy: Qt.StrongFocus
                hoverEnabled: true
                text: root.optionText(modelData)
                font.pixelSize: root.uiTheme ? root.uiTheme.bodySize : 13
                font.weight: root.currentIndex === index ? Font.DemiBold : Font.Medium
                Accessible.role: Accessible.RadioButton
                Accessible.checked: root.currentIndex === index
                onClicked: {
                    root.currentIndex = index;
                    root.optionSelected(index, root.optionValue(modelData));
                }

                contentItem: Text {
                    text: optionButton.text
                    font: optionButton.font
                    color: root.currentIndex === optionButton.index ? (root.uiTheme ? root.uiTheme.textStrong : "#12131a") : (root.uiTheme ? root.uiTheme.subtext : "#9aa4c8")
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
                background: Rectangle {
                    radius: height / 2
                    // Фон: нажата / выбрана / наведена / обычная.
                    color: optionButton.down ? (root.uiTheme ? root.uiTheme.accent2 : "#5e5ce6") : root.currentIndex === optionButton.index ? (root.uiTheme ? root.uiTheme.accent : "#0a84ff") : optionButton.hovered ? (root.uiTheme ? root.uiTheme.surfaceHigh : "#1b2040") : "transparent"
                    border.width: optionButton.activeFocus ? 2 : 0
                    border.color: root.uiTheme ? root.uiTheme.accentBright : "#5e5ce6"
                }
            }
        }
    }
}
