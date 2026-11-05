// Карточка в «стеклянном» стиле: простой прямоугольник с рамкой и скруглением.
import QtQuick

Rectangle {
    property Theme uiTheme

    color: uiTheme ? uiTheme.surface : "#121622"
    border.color: uiTheme ? uiTheme.outline : "#293146"
    border.width: 1
    radius: uiTheme ? uiTheme.radius : 16
}
