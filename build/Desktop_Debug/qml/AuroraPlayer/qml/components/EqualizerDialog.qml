// Диалог эквалайзера: оборачивает панель настройки звука в модальное окно.
import QtQuick
import QtQuick.Controls
import "../common" as Common

Common.AuroraDialog {
    id: root
    Theme {
        id: theme
    }
    uiTheme: theme
    maxDialogWidth: 790
    maxDialogHeight: 760
    title: "Эквалайзер"
    standardButtons: Dialog.Ok
    EqualizerPanel {
        width: parent.width
    }
}
