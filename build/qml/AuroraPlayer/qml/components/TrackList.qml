// Список треков с заголовками колонок и вертикальной прокруткой.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../common" as Common

Item {
    id: root
    Theme {
        id: theme
    }
    function itemAtIndex(index) {
        return rows.itemAtIndex(index);
    }
    ColumnLayout {
        anchors.fill: parent
        spacing: 4
        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 28
            Layout.leftMargin: 10
            Layout.rightMargin: 6
            spacing: 12
            visible: root.width > 850
            Item {
                Layout.preferredWidth: 44
            }
            Text {
                Layout.fillWidth: true
                Layout.preferredWidth: root.width > 900 ? Math.min(560, root.width * .46) : -1
                Layout.maximumWidth: Math.min(680, root.width * .46)
                text: "КОМПОЗИЦИЯ"
                color: theme.textMuted
                font.pixelSize: 9
                font.letterSpacing: 1.3
            }
            Text {
                Layout.preferredWidth: root.width > 1250 ? 240 : 170
                Layout.minimumWidth: Layout.preferredWidth
                Layout.fillWidth: true
                text: "АЛЬБОМ / ИСТОЧНИК"
                color: theme.textMuted
                font.pixelSize: 9
                font.letterSpacing: 1.3
            }
            Text {
                Layout.preferredWidth: 160
                Layout.minimumWidth: 160
                text: "В БИБЛИОТЕКЕ"
                color: theme.textMuted
                font.pixelSize: 9
                font.letterSpacing: 1
            }
            Item {
                Layout.preferredWidth: 32
            }
            Common.AuroraIcon {
                uiTheme: theme
                Layout.preferredWidth: 36
                name: "clock"
                size: 13
                color: theme.textMuted
            }
            Item {
                Layout.preferredWidth: 32
            }
        }
        ListView {
            id: rows
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: libraryModel
            clip: true
            spacing: 2
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: Common.AuroraScrollBar {
                uiTheme: theme
            }
            delegate: TrackDelegate {}
        }
    }
}
