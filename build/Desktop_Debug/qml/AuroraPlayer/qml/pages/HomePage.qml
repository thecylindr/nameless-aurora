// Главная страница: приветствие, «моя волна», быстрые карточки и лента рекомендаций.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../common" as Common
import "../components" as Components

Item {
    id: root
    property bool compact: false
    property var recommendations: []
    signal importRequested
    signal libraryRequested
    signal filesRequested
    signal searchRequested
    signal discoveryRequested
    Theme {
        id: theme
    }
    // Запрашивает свежую подборку рекомендаций (до 14 треков).
    function reload() {
        recommendations = libraryModel.recommendations(14);
    }
    Component.onCompleted: reload()
    Connections {
        target: libraryModel
        function onLibraryChanged() {
            root.reload();
        }
    }
    ScrollView {
        id: scroll
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ColumnLayout {
            width: scroll.availableWidth - (root.compact ? 32 : 64)
            x: root.compact ? 16 : 32
            spacing: 26
            Item {
                implicitHeight: 6
            }
            RowLayout {
                Layout.fillWidth: true
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 7
                    Text {
                        text: "ТВОЁ ЛИЧНОЕ ПРОСТРАНСТВО"
                        color: theme.subtext
                        font.pixelSize: 10
                        font.letterSpacing: 2
                    }
                    Text {
                        text: "Главная"
                        color: theme.text
                        font.pixelSize: 32
                        font.weight: Font.DemiBold
                    }
                }
                Common.AuroraButton {
                    uiTheme: theme
                    iconName: "plus"
                    text: root.compact ? "" : "Добавить музыку"
                    iconOnly: root.compact
                    onClicked: root.filesRequested()
                }
            }
            Rectangle {
                id: hero
                visible: settingsStore.homeHeroVisible
                Layout.fillWidth: true
                implicitHeight: root.compact ? 300 : 292
                radius: 24
                clip: true
                color: theme.surfaceHigh
                Components.AmbientBackdrop {
                    anchors.fill: parent
                    animated: true
                }
                Components.AtmosphereOrb {
                    width: Math.min(parent.width * 0.72, 430)
                    height: width
                    anchors.right: parent.right
                    anchors.rightMargin: root.compact ? -100 : -6
                    anchors.verticalCenter: parent.verticalCenter
                    opacity: root.compact ? 0.65 : 1
                }
                Components.CornerCover {
                    anchors.fill: parent
                    radius: 24
                    color: theme.base
                    z: 5
                }
                ColumnLayout {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.margins: root.compact ? 24 : 32
                    width: parent.width * (root.compact ? 0.85 : 0.62)
                    spacing: 12
                    Text {
                        text: "НА ТВОЕЙ ЧАСТОТЕ"
                        color: theme.accent
                        font.pixelSize: 10
                        font.letterSpacing: 2.4
                    }
                    Item {
                        Layout.fillHeight: true
                    }
                    Text {
                        text: "Моя волна"
                        color: "#ffffff"
                        font.pixelSize: root.compact ? 40 : 48
                        font.weight: Font.DemiBold
                        font.letterSpacing: -1.8
                    }
                    Text {
                        Layout.fillWidth: true
                        text: "Знакомые треки. Новое настроение.\nПодборка, которая слышит тебя."
                        color: theme.subtext
                        font.pixelSize: 14
                        lineHeight: 1.4
                        wrapMode: Text.WordWrap
                    }
                    Item {
                        implicitHeight: 5
                    }
                    Common.AuroraButton {
                        uiTheme: theme
                        primary: true
                        iconName: "play"
                        text: libraryModel.totalCount ? "Включить волну" : "Добавить первые треки"
                        implicitHeight: 46
                        onClicked: libraryModel.totalCount ? player.playMix() : root.filesRequested()
                    }
                }
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                Common.AuroraButton {
                    uiTheme: theme
                    Layout.fillWidth: true
                    text: "Найти музыку"
                    iconName: "search"
                    outlined: true
                    onClicked: root.searchRequested()
                }
                Common.AuroraButton {
                    uiTheme: theme
                    Layout.fillWidth: true
                    text: "По моему вкусу"
                    iconName: "shuffle"
                    onClicked: root.discoveryRequested()
                }
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: 12
                Repeater {
                    model: [
                        {
                            name: "Мне нравится",
                            sub: "Любимое рядом",
                            icon: "heart",
                            tint: "#67529a"
                        },
                        {
                            name: "Моя коллекция",
                            sub: "Вся музыка с собой",
                            icon: "library",
                            tint: "#386d69"
                        }
                    ]
                    delegate: Rectangle {
                        required property var modelData
                        required property int index
                        Layout.fillWidth: true
                        implicitHeight: root.compact ? 88 : 84
                        radius: 14
                        color: tileHover.containsMouse ? theme.hover : theme.surface
                        border.color: theme.line
                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 14
                            Rectangle {
                                Layout.preferredWidth: 48
                                Layout.preferredHeight: 48
                                radius: 12
                                color: modelData.tint
                                visible: !root.compact
                                Common.AuroraIcon {
                                    anchors.centerIn: parent
                                    name: modelData.icon
                                    size: 22
                                    color: "#f6edff"
                                }
                            }
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 6
                                Text {
                                    Layout.fillWidth: true
                                    text: modelData.name
                                    color: theme.text
                                    font.pixelSize: root.compact ? 13 : 15
                                    font.weight: Font.Medium
                                    elide: Text.ElideRight
                                }
                                Text {
                                    Layout.fillWidth: true
                                    text: modelData.sub
                                    color: theme.subtext
                                    font.pixelSize: 11
                                    elide: Text.ElideRight
                                }
                            }
                        }
                        MouseArea {
                            id: tileHover
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            // Первая карточка ведёт в «Мне нравится» (-1), вторая — во всю коллекцию (0).
                            onClicked: {
                                libraryModel.playlistFilter = index === 0 ? -1 : 0;
                                root.libraryRequested();
                            }
                        }
                    }
                }
            }
            RowLayout {
                Layout.fillWidth: true
                Text {
                    text: root.recommendations.length ? "В твоём ритме" : "Вся музыка — в одном месте"
                    color: theme.text
                    font.pixelSize: 22
                    font.weight: Font.DemiBold
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                }
                Common.AuroraButton {
                    uiTheme: theme
                    text: "Открыть всё"
                    visible: root.recommendations.length > 0
                    implicitHeight: 34
                    onClicked: {
                        libraryModel.playlistFilter = 0;
                        root.libraryRequested();
                    }
                }
            }
            ListView {
                Layout.fillWidth: true
                implicitHeight: 232
                visible: root.recommendations.length > 0
                orientation: ListView.Horizontal
                model: root.recommendations
                spacing: 16
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                delegate: Components.RecommendationCard {
                    required property var modelData
                    trackData: modelData
                    onClicked: player.playTrackId(modelData.trackId)
                }
            }
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: emptyContent.implicitHeight + 40
                visible: root.recommendations.length === 0
                color: theme.surface
                radius: 16
                border.color: theme.line
                ColumnLayout {
                    id: emptyContent
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 20
                    spacing: 16
                    Text {
                        Layout.fillWidth: true
                        text: "Перенеси свои плейлисты или добавь музыку с устройства. Aurora сохранит коллекцию, а волна подстроится под твои оценки и прослушивания."
                        color: theme.subtext
                        font.pixelSize: 14
                        wrapMode: Text.WordWrap
                        lineHeight: 1.4
                    }
                    Flow {
                        Layout.fillWidth: true
                        spacing: 8
                        Repeater {
                            model: ["Яндекс Музыка", "VK Музыка", "YouTube"]
                            delegate: Common.AuroraButton {
                                required property string modelData
                                uiTheme: theme
                                text: modelData
                                implicitHeight: 38
                                onClicked: root.importRequested()
                            }
                        }
                    }
                }
            }
            Text {
                text: "ТВОЯ МУЗЫКА. ТВОИ ПРАВИЛА.  /  NAMELESS AURORA"
                color: theme.textMuted
                font.pixelSize: 9
                font.letterSpacing: 1.5
                Layout.bottomMargin: 24
            }
        }
    }
}
