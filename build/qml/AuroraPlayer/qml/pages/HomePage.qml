import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../common" as Common
import "../components" as Components

Item {
    id: root
    property bool compact: false
    property var recommendationModel: []
    signal importRequested()
    signal libraryRequested()
    Theme { id: theme }

    function reloadRecommendations() {
        recommendationModel = libraryModel.recommendations(compact ? 10 : 14)
    }
    function greeting() {
        const hour = new Date().getHours()
        if (hour < 6) return "Доброй ночи"
        if (hour < 12) return "Доброе утро"
        if (hour < 18) return "Добрый день"
        return "Добрый вечер"
    }

    Component.onCompleted: reloadRecommendations()
    Connections {
        target: libraryModel
        function onLibraryChanged() { root.reloadRecommendations() }
    }

    ScrollView {
        id: scroll
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth
        background: Item {}
        ScrollBar.vertical: Common.AuroraScrollBar { uiTheme: theme; x: scroll.width - width; height: scroll.height }

        ColumnLayout {
            width: Math.min(1120, Math.max(280, scroll.availableWidth - (compact ? 32 : 56)))
            x: Math.max(compact ? 16 : 28, (scroll.availableWidth - width) / 2)
            spacing: 22

            Item { implicitHeight: compact ? 16 : 28 }
            RowLayout {
                Layout.fillWidth: true
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 3
                    Text { text: greeting(); color: theme.text; font.pixelSize: compact ? 25 : 32; font.weight: Font.DemiBold }
                    Text { text: "Ваше музыкальное пространство"; color: theme.subtext; font.pixelSize: theme.bodySize }
                }
                Common.AuroraButton {
                    uiTheme: theme
                    iconOnly: true
                    iconName: "refresh"
                    implicitWidth: 42
                    onClicked: root.reloadRecommendations()
                }
            }

            Common.AuroraCard {
                uiTheme: theme
                Layout.fillWidth: true
                implicitHeight: compact ? 218 : 190
                padding: 0
                clip: true
                Rectangle {
                    anchors.fill: parent
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0; color: theme.accent2 }
                        GradientStop { position: 1; color: theme.surface }
                    }
                }
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: compact ? 18 : 24
                    spacing: 20
                    Rectangle {
                        Layout.preferredWidth: compact ? 92 : 138
                        Layout.preferredHeight: compact ? 92 : 138
                        Layout.alignment: Qt.AlignVCenter
                        radius: theme.radius
                        color: theme.surfaceHigh
                        clip: true
                        Image {
                            anchors.fill: parent
                            source: player.currentCoverPath ? encodeURI("file://" + player.currentCoverPath) : ""
                            fillMode: Image.PreserveAspectCrop
                            asynchronous: true
                        }
                        Common.AuroraIcon { anchors.centerIn: parent; name: "music"; color: theme.text; size: 42; visible: player.currentCoverPath.length === 0 }
                    }
                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter
                        spacing: 7
                        Text { text: player.hasTrack ? "ПРОДОЛЖИТЬ" : "СОБЕРИТЕ СВОЙ МИР"; color: theme.text; font.pixelSize: 10; font.weight: Font.DemiBold }
                        Text {
                            Layout.fillWidth: true
                            text: player.hasTrack ? player.currentTitle : "Музыка начинается локально"
                            color: "white"
                            font.pixelSize: compact ? 19 : 26
                            font.weight: Font.DemiBold
                            elide: Text.ElideRight
                        }
                        Text {
                            Layout.fillWidth: true
                            text: player.hasTrack ? player.currentArtist : "Импортируйте файлы или свои плейлисты — рекомендации появятся по мере прослушивания"
                            color: theme.text
                            opacity: 0.8
                            font.pixelSize: theme.captionSize
                            wrapMode: Text.WordWrap
                            maximumLineCount: 2
                            elide: Text.ElideRight
                        }
                        Common.AuroraButton {
                            uiTheme: theme
                            primary: true
                            iconName: player.hasTrack ? "play" : "plus"
                            text: player.hasTrack ? "Слушать" : "Добавить музыку"
                            implicitHeight: 40
                            onClicked: player.hasTrack ? player.toggle() : root.importRequested()
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                visible: recommendationModel.length > 0
                spacing: 12
                RowLayout {
                    Layout.fillWidth: true
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        Text { text: "Для вас"; color: theme.text; font.pixelSize: 22; font.weight: Font.DemiBold }
                        Text { text: "Локальный вкусовой профиль · без облака"; color: theme.subtext; font.pixelSize: theme.captionSize }
                    }
                    Common.AuroraButton { uiTheme: theme; text: "Вся музыка"; implicitHeight: 38; onClicked: root.libraryRequested() }
                }
                ListView {
                    Layout.fillWidth: true
                    implicitHeight: 236
                    orientation: ListView.Horizontal
                    model: root.recommendationModel
                    spacing: 12
                    clip: true
                    boundsBehavior: Flickable.StopAtBounds
                    delegate: Components.RecommendationCard {
                        required property var modelData
                        trackData: modelData
                        onClicked: player.playTrackId(modelData.trackId)
                    }
                }
            }

            Common.AuroraCard {
                uiTheme: theme
                Layout.fillWidth: true
                visible: recommendationModel.length === 0
                implicitHeight: emptyContent.implicitHeight + 36
                padding: 18
                ColumnLayout {
                    id: emptyContent
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.margins: 18
                    spacing: 10
                    Common.AuroraIcon { name: "music"; color: theme.accent; size: 34 }
                    Text { text: "Здесь появятся ваши подборки"; color: theme.text; font.pixelSize: 19; font.weight: Font.Medium }
                    Text {
                        Layout.fillWidth: true
                        text: "Aurora учитывает только локальные оценки, прослушивания и пропуски. Ничего не отправляется на сервер."
                        color: theme.subtext
                        font.pixelSize: theme.bodySize
                        wrapMode: Text.WordWrap
                    }
                    Common.AuroraButton { uiTheme: theme; primary: true; text: "Открыть импорт"; iconName: "download"; onClicked: root.importRequested() }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                visible: recommendationModel.length > 0
                spacing: 12
                Text { text: "Быстрый выбор"; color: theme.text; font.pixelSize: 22; font.weight: Font.DemiBold }
                GridLayout {
                    Layout.fillWidth: true
                    columns: compact ? 1 : 2
                    columnSpacing: 12
                    rowSpacing: 10
                    Repeater {
                        model: root.recommendationModel.slice(0, compact ? 4 : 6)
                        delegate: Common.AuroraCard {
                            required property var modelData
                            uiTheme: theme
                            Layout.fillWidth: true
                            implicitHeight: 68
                            padding: 6
                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 6
                                spacing: 10
                                Rectangle {
                                    Layout.preferredWidth: 50
                                    Layout.preferredHeight: 50
                                    radius: 10
                                    color: theme.accent2
                                    clip: true
                                    Image { anchors.fill: parent; source: modelData.coverPath ? encodeURI("file://" + modelData.coverPath) : ""; fillMode: Image.PreserveAspectCrop; asynchronous: true }
                                    Common.AuroraIcon { anchors.centerIn: parent; name: "music"; color: theme.text; size: 20; visible: !modelData.coverPath }
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 2
                                    Text { Layout.fillWidth: true; text: modelData.title; color: theme.text; font.pixelSize: theme.bodySize; elide: Text.ElideRight }
                                    Text { Layout.fillWidth: true; text: modelData.artist; color: theme.subtext; font.pixelSize: theme.captionSize; elide: Text.ElideRight }
                                }
                                Common.AuroraButton { uiTheme: theme; iconOnly: true; iconName: "play"; implicitWidth: 38; implicitHeight: 38; onClicked: player.playTrackId(modelData.trackId) }
                            }
                        }
                    }
                }
            }
            Item { implicitHeight: 28 }
        }
    }
}
