// Поле ввода текста с опциональной иконкой (поиск или произвольная).
import QtQuick
import QtQuick.Controls
import ".."

TextField {
    id: control
    property Theme uiTheme
    // Показывать ли иконку-лупу (поиск).
    property bool searchIcon: false
    // Имя произвольной иконки слева.
    property string iconName: ""

    implicitHeight: uiTheme ? uiTheme.controlHeight : 44
    implicitWidth: 220
    // Дополнительный отступ слева под иконку.
    leftPadding: searchIcon || iconName.length > 0 ? 42 : 14
    rightPadding: 14
    topPadding: 0
    bottomPadding: 0
    verticalAlignment: TextInput.AlignVCenter
    color: uiTheme ? uiTheme.text : "#eaf2ff"
    placeholderTextColor: uiTheme ? uiTheme.subtext : "#9aa4c8"
    selectionColor: uiTheme ? uiTheme.accent : "#0a84ff"
    selectedTextColor: "#ffffff"
    font.pixelSize: uiTheme ? uiTheme.bodySize : 13
    focusPolicy: Qt.StrongFocus

    AuroraIcon {
        visible: control.searchIcon || control.iconName.length > 0
        uiTheme: control.uiTheme
        // Если имя иконки не задано — используем поиск по умолчанию.
        name: control.iconName.length > 0 ? control.iconName : "search"
        size: 18
        // Иконка подсвечивается акцентом при фокусе.
        color: control.activeFocus ? (control.uiTheme ? control.uiTheme.accent : "#0a84ff") : (control.uiTheme ? control.uiTheme.subtext : "#9aa4c8")
        anchors.left: parent.left
        anchors.leftMargin: 14
        anchors.verticalCenter: parent.verticalCenter
        z: 1
    }

    background: Rectangle {
        radius: control.uiTheme ? control.uiTheme.radiusSmall : 10
        // Фон меняется для отключённого состояния.
        color: control.enabled ? (control.uiTheme ? control.uiTheme.surface : "#121626") : (control.uiTheme ? control.uiTheme.base : "#05070f")
        border.width: control.activeFocus ? 2 : 1
        // Рамка: акцент при фокусе, иначе — подсказка при наведении или обычная линия.
        border.color: control.activeFocus ? (control.uiTheme ? control.uiTheme.accent : "#0a84ff") : control.hovered ? (control.uiTheme ? control.uiTheme.subtext : "#9aa4c8") : (control.uiTheme ? control.uiTheme.line : "#1effffff")
    }
}
