import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "common" as Common
import "components" as Components
import "pages" as Pages

ApplicationWindow {
    id: root
    visible: true
    width: 1360
    height: 840
    minimumWidth: 360
    minimumHeight: 560
    title: "Aurora Player"
    color: theme.background

    property int currentPage: 0
    readonly property bool compact: width < 820
    readonly property real safeTop: safeArea.SafeArea.margins.top
    readonly property real safeBottom: safeArea.SafeArea.margins.bottom
    readonly property real safeLeft: safeArea.SafeArea.margins.left
    readonly property real safeRight: safeArea.SafeArea.margins.right

    Theme { id: theme }
    Item { id: safeArea; anchors.fill: parent }

    palette.window: theme.background
    palette.windowText: theme.text
    palette.base: theme.surface
    palette.text: theme.text
    palette.button: theme.surfaceHigh
    palette.buttonText: theme.text
    palette.highlight: theme.accent
    palette.highlightedText: theme.textStrong
    palette.placeholderText: theme.subtext

    onClosing: close => {
        if (Qt.platform.os === "android" && currentPage !== 0) {
            close.accepted = false
            currentPage = 0
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: safeLeft
        anchors.rightMargin: safeRight
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: safeTop
            spacing: 0

            Components.NavigationPane {
                visible: !compact
                Layout.preferredWidth: 220
                Layout.fillHeight: true
                currentPage: root.currentPage
                onPageSelected: page => root.currentPage = page
            }

            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: root.currentPage

                Pages.HomePage {
                    compact: root.compact
                    onImportRequested: root.currentPage = 2
                    onLibraryRequested: root.currentPage = 1
                }
                Pages.LibraryPage {
                    compact: root.compact
                    onImportUrlRequested: root.currentPage = 2
                }
                Pages.ImportPage { compact: root.compact }
                Pages.SettingsPage { compact: root.compact }
            }

            Components.NowPlayingPanel {
                visible: !compact && root.width >= 1240 && root.currentPage <= 1
                Layout.preferredWidth: 270
                Layout.fillHeight: true
                Layout.margins: 24
                Layout.leftMargin: 0
            }
        }

        Components.PlayerBar {
            Layout.fillWidth: true
            compact: root.compact
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: compact ? 64 + safeBottom : 0
            visible: compact
            color: theme.sidebar
            border.color: theme.line
            border.width: 1
            RowLayout {
                anchors.fill: parent
                anchors.bottomMargin: safeBottom
                spacing: 0
                Repeater {
                    model: [
                        { label: "Главная", icon: "music" },
                        { label: "Музыка", icon: "library" },
                        { label: "Импорт", icon: "download" },
                        { label: "Опции", icon: "settings" }
                    ]
                    delegate: Common.AuroraButton {
                        required property int index
                        required property var modelData
                        uiTheme: theme
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: modelData.label
                        iconName: modelData.icon
                        primary: root.currentPage === index
                        onClicked: root.currentPage = index
                    }
                }
            }
        }
    }
}
