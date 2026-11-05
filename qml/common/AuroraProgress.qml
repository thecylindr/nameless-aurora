// Индикатор прогресса с детерминированной полосой и бегущим индикатором для indeterminate.
import QtQuick
import QtQuick.Controls
import ".."

ProgressBar {
    id: control
    property Theme uiTheme
    // Анимация активна только когда компонент видим, окно не свёрнуто и движение включено в настройках.
    readonly property bool motionEnabled: visible && enabled && width > 0 && (!uiTheme || uiTheme.motionEnabled) && Window.window !== null && Window.window.visibility !== Window.Minimized && Window.window.visibility !== Window.Hidden

    implicitWidth: 180
    implicitHeight: 12
    padding: 0
    opacity: enabled ? 1 : 0.45

    background: Rectangle {
        id: track
        // Дорожка центрируется по вертикали внутри доступной области.
        x: control.leftPadding
        y: control.topPadding + (control.availableHeight - height) / 2
        width: control.availableWidth
        height: 6
        radius: 3
        color: control.uiTheme ? control.uiTheme.surfaceHigh : "#1b2040"
    }

    contentItem: Item {
        implicitHeight: 6
        clip: true

        Rectangle {
            visible: !control.indeterminate
            // Ширина заполнения пропорциональна текущей позиции прогресса.
            width: control.visualPosition * parent.width
            height: parent.height
            radius: height / 2
            color: control.uiTheme ? control.uiTheme.accent : "#0a84ff"
            Behavior on width {
                enabled: control.motionEnabled
                NumberAnimation {
                    duration: 120
                }
            }
        }

        Rectangle {
            id: movingIndicator
            visible: control.indeterminate
            // Ширина бегущего индикатора — не менее 48 и не более 30% дорожки.
            width: Math.max(48, parent.width * 0.3)
            height: parent.height
            radius: height / 2
            color: control.uiTheme ? control.uiTheme.accent : "#0a84ff"
            NumberAnimation on x {
                id: progressAnimation
                running: control.indeterminate && control.motionEnabled
                loops: Animation.Infinite
                // Индикатор пробегает от левого края до правого и назад.
                from: -movingIndicator.width
                to: movingIndicator.parent.width
                duration: 1100
                easing.type: Easing.InOutCubic
            }
            Binding {
                target: movingIndicator
                property: "x"
                // Когда анимация не идёт, удерживаем индикатор по центру.
                value: (movingIndicator.parent.width - movingIndicator.width) / 2
                when: !progressAnimation.running
            }
        }
    }
}
