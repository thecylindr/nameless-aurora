// Контекстное/выпадающее меню в стиле Aurora с кастомным делегатом пунктов.
import QtQuick
import QtQuick.Controls
import ".."

Menu {
    id: control
    required property Theme uiTheme
    // Меню привязано к элементу (не является отдельным окном).
    popupType: Popup.Item
    width: 274
    padding: 6
    // Синхронизируем системную палитру с темой, чтобы цвет текста совпадал.
    palette.windowText: uiTheme.text
    palette.text: uiTheme.text
    palette.highlightedText: uiTheme.text
    palette.highlight: uiTheme.selected
    delegate: AuroraMenuItem {
        uiTheme: control.uiTheme
    }
    background: Rectangle {
        radius: 16
        color: control.uiTheme.surfaceHigh
        border.color: control.uiTheme.line
    }
}
