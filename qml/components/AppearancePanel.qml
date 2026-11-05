// Панель настройки оформления: палитры, фоновые сцены, реакция на музыку и компоновка главного окна.
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import "../common" as Common

ColumnLayout {
    id: root
    spacing: 16
    Theme {
        id: theme
    }
    // Живой предпросмотр выбранной атмосферы поверх карточки-заголовка.
    Rectangle {
        Layout.fillWidth: true
        implicitHeight: root.width < 500 ? 215 : 245
        radius: 22
        clip: true
        color: theme.surfaceHigh
        AmbientBackdrop {
            anchors.fill: parent
            tint: theme.accent
            secondary: theme.accent2
            animated: true
        }
        AtmosphereOrb {
            width: root.width < 500 ? 230 : 330
            height: width
            anchors.right: parent.right
            anchors.rightMargin: -35
            anchors.verticalCenter: parent.verticalCenter
            interactive: false
            primary: theme.accent
            secondary: theme.accent2
        }
        CornerCover {
            anchors.fill: parent
            radius: 18
            color: theme.base
            z: 5
        }
        Column {
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width * 0.57
            spacing: 9
            Text {
                text: "ТВОЯ АТМОСФЕРА"
                color: theme.accent
                font.pixelSize: 9
                font.letterSpacing: 2
            }
            Text {
                text: "Настрой\nсвоё пространство"
                width: parent.width
                wrapMode: Text.WordWrap
                color: theme.text
                font.pixelSize: root.width < 500 ? 23 : 31
                font.weight: Font.DemiBold
                lineHeight: 1.1
            }
            Text {
                text: "Цвет, свет и движение\nв твоём ритме"
                color: theme.subtext
                font.pixelSize: 12
                lineHeight: 1.4
            }
        }
    }
    Text {
        text: "Цвет пространства"
        color: theme.text
        font.pixelSize: 14
        font.weight: Font.Medium
    }
    GridLayout {
        Layout.fillWidth: true
        columns: width < 430 ? 2 : 3
        rowSpacing: 10
        columnSpacing: 10
        Repeater {
            // Набор готовых цветовых схем; последняя подхватывает акцент из NameLess UI.
            model: [
                {
                    id: "aurora",
                    name: "Аврора",
                    a: "#baa5ff",
                    b: "#83dbc7",
                    base: "#171a25"
                },
                {
                    id: "lagoon",
                    name: "Лагуна",
                    a: "#7fdfcb",
                    b: "#7aaff3",
                    base: "#121f25"
                },
                {
                    id: "ember",
                    name: "Тёплый вечер",
                    a: "#fbb38d",
                    b: "#e2a2be",
                    base: "#241a19"
                },
                {
                    id: "rose",
                    name: "Сакура",
                    a: "#eba8cc",
                    b: "#b2acff",
                    base: "#241a29"
                },
                {
                    id: "midnight",
                    name: "Полночь",
                    a: "#8ab8ff",
                    b: "#afa6ff",
                    base: "#131d2d"
                },
                {
                    id: "nameless",
                    name: "NameLess UI",
                    a: namelessBridge.accent,
                    b: namelessBridge.secondary,
                    base: "#171a25"
                }
            ]
            delegate: Rectangle {
                id: option
                required property var modelData
                Layout.fillWidth: true
                Layout.minimumWidth: 90
                implicitHeight: 112
                radius: 16
                color: modelData.base
                border.width: settingsStore.colorScheme === modelData.id ? 2 : 1
                border.color: settingsStore.colorScheme === modelData.id ? modelData.a : theme.line
                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 10
                    height: 57
                    radius: 10
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop {
                            position: 0
                            color: option.modelData.a
                        }
                        GradientStop {
                            position: 1
                            color: option.modelData.b
                        }
                    }
                    Rectangle {
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.leftMargin: 10
                        width: 25
                        height: 25
                        radius: 8
                        color: option.modelData.base
                        opacity: .85
                    }
                    Rectangle {
                        anchors.left: parent.left
                        anchors.leftMargin: 44
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width - 58
                        height: 4
                        radius: 2
                        color: option.modelData.base
                        opacity: .4
                    }
                }
                Text {
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    anchors.margins: 14
                    text: modelData.name
                    color: theme.text
                    font.pixelSize: 12
                }
                Common.AuroraIcon {
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.margins: 13
                    name: "check"
                    size: 16
                    color: option.modelData.a
                    visible: settingsStore.colorScheme === option.modelData.id
                }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: settingsStore.colorScheme = option.modelData.id
                }
                Accessible.role: Accessible.Button
                Accessible.name: modelData.name
                Accessible.onPressAction: settingsStore.colorScheme = modelData.id
                activeFocusOnTab: true
                Keys.onSpacePressed: settingsStore.colorScheme = modelData.id
                Keys.onReturnPressed: settingsStore.colorScheme = modelData.id
            }
        }
    }
    RowLayout {
        Layout.fillWidth: true
        Rectangle {
            width: 24
            height: 24
            radius: 12
            color: settingsStore.customAccent
            border.color: theme.line
        }
        Common.AuroraButton {
            uiTheme: theme
            Layout.fillWidth: true
            text: "Свой цвет · " + settingsStore.customAccent.toUpperCase()
            outlined: settingsStore.colorScheme === "custom"
            onClicked: customColor.open()
        }
    }
    Text {
        text: "Сцена за музыкой"
        color: theme.text
        font.pixelSize: 14
        font.weight: Font.Medium
    }
    GridLayout {
        Layout.fillWidth: true
        columns: width < 520 ? 2 : 3
        rowSpacing: 8
        columnSpacing: 8
        Repeater {
            // Перечень фоновых сцен за музыкой, из которых пользователь выбирает одну.
            model: [
                {
                    id: "orb",
                    name: "Сфера",
                    symbol: "◉",
                    note: "Только свет и форма"
                },
                {
                    id: "cosmos",
                    name: "Космос",
                    symbol: "✧",
                    note: "Орбиты и созвездия"
                },
                {
                    id: "prisms",
                    name: "Призмы",
                    symbol: "△",
                    note: "Парящая геометрия"
                },
                {
                    id: "bubbles",
                    name: "Пузырьки",
                    symbol: "○",
                    note: "Лёгкость и глубина"
                },
                {
                    id: "meteors",
                    name: "Метеоры",
                    symbol: "☄",
                    note: "Световые следы"
                },
                {
                    id: "ribbons",
                    name: "Шёлк",
                    symbol: "≈",
                    note: "Парящие ленты света"
                },
                {
                    id: "bonsai",
                    name: "Бонсай",
                    symbol: "♧",
                    note: "Большое дерево и живой свет"
                },
                {
                    id: "off",
                    name: "Тишина",
                    symbol: "—",
                    note: "Без фоновых эффектов"
                }
            ]
            delegate: Rectangle {
                required property var modelData
                Layout.fillWidth: true
                implicitHeight: 98
                radius: 14
                color: settingsStore.atmosphereScene === modelData.id ? theme.selected : theme.surfaceHigh
                border.color: settingsStore.atmosphereScene === modelData.id ? theme.accent : theme.line
                Column {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 5
                    Text {
                        text: modelData.symbol + "  " + modelData.name
                        color: theme.text
                        font.pixelSize: 15
                    }
                    Text {
                        width: parent.width
                        text: modelData.note
                        color: theme.subtext
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                    }
                }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: settingsStore.atmosphereScene = modelData.id
                }
                Accessible.role: Accessible.Button
                Accessible.name: modelData.name
                Accessible.onPressAction: settingsStore.atmosphereScene = modelData.id
                activeFocusOnTab: true
                Keys.onReturnPressed: settingsStore.atmosphereScene = modelData.id
                Keys.onSpacePressed: settingsStore.atmosphereScene = modelData.id
            }
        }
    }
    RowLayout {
        Layout.fillWidth: true
        Text {
            text: "Количество объектов"
            color: theme.subtext
            font.pixelSize: 12
            Layout.fillWidth: true
        }
        Text {
            text: Math.round(settingsStore.objectDensity * 100) + "%"
            color: theme.accent
            font.pixelSize: 12
        }
    }
    Common.AuroraSlider {
        uiTheme: theme
        Layout.fillWidth: true
        from: 0
        to: 1
        stepSize: .05
        value: settingsStore.objectDensity
        enabled: ["orb", "off"].indexOf(settingsStore.atmosphereScene) < 0
        onMoved: settingsStore.objectDensity = value
    }
    RowLayout {
        Layout.fillWidth: true
        Text {
            text: "Размер декораций"
            color: theme.subtext
            font.pixelSize: 12
            Layout.fillWidth: true
        }
        Text {
            text: Math.round(settingsStore.objectScale * 100) + "%"
            color: theme.accent
            font.pixelSize: 12
        }
    }
    Common.AuroraSlider {
        uiTheme: theme
        Layout.fillWidth: true
        from: 0.5
        to: 1.8
        stepSize: 0.05
        value: settingsStore.objectScale
        onMoved: settingsStore.objectScale = value
    }
    RowLayout {
        Layout.fillWidth: true
        Text {
            text: "Видимость декораций"
            color: theme.subtext
            font.pixelSize: 12
            Layout.fillWidth: true
        }
        Text {
            text: Math.round(settingsStore.decorationOpacity * 100) + "%"
            color: theme.accent
            font.pixelSize: 12
        }
    }
    Common.AuroraSlider {
        uiTheme: theme
        Layout.fillWidth: true
        from: 0
        to: 1
        stepSize: 0.05
        value: settingsStore.decorationOpacity
        onMoved: settingsStore.decorationOpacity = value
    }
    RowLayout {
        Layout.fillWidth: true
        Text {
            text: "Скорость движения"
            color: theme.subtext
            font.pixelSize: 12
            Layout.fillWidth: true
        }
        Text {
            text: settingsStore.effectSpeed.toFixed(2) + "×"
            color: theme.accent
            font.pixelSize: 12
        }
    }
    Common.AuroraSlider {
        uiTheme: theme
        Layout.fillWidth: true
        from: .25
        to: 2
        stepSize: .05
        value: settingsStore.effectSpeed
        enabled: settingsStore.motionEnabled
        onMoved: settingsStore.effectSpeed = value
    }
    Text {
        visible: ["orb", "cosmos"].indexOf(settingsStore.atmosphereScene) >= 0
        text: "Характер сферы"
        color: theme.text
        font.pixelSize: 14
        font.weight: Font.Medium
    }
    Common.AuroraSegmented {
        visible: ["orb", "cosmos"].indexOf(settingsStore.atmosphereScene) >= 0
        uiTheme: theme
        Layout.fillWidth: true
        // Три стиля сферы маппятся на внутренние значения ribbons / orbit / liquid.
        model: ["Сияние", "Орбиты", "Поток"]
        currentIndex: ["ribbons", "orbit", "liquid"].indexOf(settingsStore.sphereStyle)
        onOptionSelected: index => settingsStore.sphereStyle = ["ribbons", "orbit", "liquid"][index]
    }
    RowLayout {
        visible: ["orb", "cosmos"].indexOf(settingsStore.atmosphereScene) >= 0
        Layout.fillWidth: true
        Text {
            text: "Изменение формы сферы"
            color: theme.subtext
            font.pixelSize: 12
            Layout.fillWidth: true
        }
        Text {
            text: Math.round(settingsStore.sphereDeformation * 100) + "%"
            color: theme.accent
            font.pixelSize: 12
        }
    }
    Common.AuroraSlider {
        visible: ["orb", "cosmos"].indexOf(settingsStore.atmosphereScene) >= 0
        uiTheme: theme
        Layout.fillWidth: true
        from: 0
        to: 3
        stepSize: 0.05
        value: settingsStore.sphereDeformation
        enabled: settingsStore.motionEnabled
        onMoved: settingsStore.sphereDeformation = value
    }
    Common.AuroraSwitch {
        uiTheme: theme
        Layout.fillWidth: true
        text: "Живая атмосфера"
        subtext: "Плавное движение света и сферы"
        checked: settingsStore.motionEnabled
        onToggled: settingsStore.motionEnabled = checked
    }
    Common.AuroraSwitch {
        uiTheme: theme
        Layout.fillWidth: true
        text: "Импульсы на ударах"
        subtext: "Волны света, искры и изменение формы на акцентах музыки"
        checked: settingsStore.beatEffects
        enabled: settingsStore.motionEnabled
        onToggled: settingsStore.beatEffects = checked
    }
    RowLayout {
        Layout.fillWidth: true
        Text {
            text: "Отклик на музыку"
            color: theme.subtext
            font.pixelSize: 12
            Layout.fillWidth: true
        }
        Text {
            text: Math.round(settingsStore.reactionStrength * 100) + "%"
            color: theme.accent
            font.pixelSize: 12
        }
    }
    Common.AuroraSlider {
        uiTheme: theme
        Layout.fillWidth: true
        from: 0
        to: 2
        stepSize: .05
        value: settingsStore.reactionStrength
        enabled: settingsStore.motionEnabled
        onMoved: settingsStore.reactionStrength = value
    }
    RowLayout {
        Layout.fillWidth: true
        Text {
            text: "Интенсивность свечения"
            color: theme.subtext
            font.pixelSize: 12
            Layout.fillWidth: true
        }
        Text {
            text: Math.round(settingsStore.atmosphereIntensity * 100) + "%"
            color: theme.accent
            font.pixelSize: 12
        }
    }
    Common.AuroraSlider {
        uiTheme: theme
        Layout.fillWidth: true
        from: 0
        to: 1.5
        stepSize: 0.05
        value: settingsStore.atmosphereIntensity
        onMoved: settingsStore.atmosphereIntensity = value
    }
    Text {
        Layout.fillWidth: true
        text: "Сфера реагирует на бас и удары. Отключение движения оставляет статичный свет и снижает нагрузку."
        color: theme.textMuted
        font.pixelSize: 12
        wrapMode: Text.WordWrap
        lineHeight: 1.4
    }
    Common.AuroraButton {
        uiTheme: theme
        text: "Вернуть оформление Aurora"
        iconName: "refresh"
        // Возвращает оформление к исходным значениям Aurora по умолчанию.
        onClicked: {
            settingsStore.colorScheme = "aurora";
            settingsStore.customAccent = "#b9a1ff";
            settingsStore.sphereStyle = "ribbons";
            settingsStore.motionEnabled = true;
            settingsStore.atmosphereIntensity = .85;
            settingsStore.beatEffects = true;
            settingsStore.reactionStrength = 1.15;
            settingsStore.atmosphereScene = "cosmos";
            settingsStore.objectDensity = .55;
            settingsStore.effectSpeed = 1.;
            settingsStore.sphereDeformation = 1.;
            settingsStore.objectScale = 1.;
            settingsStore.decorationOpacity = 1.;
        }
    }
    Rectangle {
        Layout.fillWidth: true
        implicitHeight: 1
        color: theme.line
        Layout.topMargin: 8
        Layout.bottomMargin: 8
    }
    Text {
        text: "Главное окно"
        color: theme.text
        font.pixelSize: 18
        font.weight: Font.DemiBold
    }
    Text {
        Layout.fillWidth: true
        text: "Собери рабочее пространство под себя. Переключатели действуют сразу и сохраняются после закрытия."
        color: theme.subtext
        font.pixelSize: 12
        wrapMode: Text.WordWrap
    }
    Flow {
        Layout.fillWidth: true
        spacing: 8
        Common.AuroraButton {
            uiTheme: theme
            text: "Студия"
            onClicked: {
                settingsStore.compactNavigation = false;
                settingsStore.sideQueueVisible = true;
                settingsStore.compactPlayer = false;
                settingsStore.homeHeroVisible = true;
            }
        }
        Common.AuroraButton {
            uiTheme: theme
            text: "Фокус"
            onClicked: {
                settingsStore.compactNavigation = true;
                settingsStore.sideQueueVisible = false;
                settingsStore.compactPlayer = false;
                settingsStore.homeHeroVisible = true;
            }
        }
        Common.AuroraButton {
            uiTheme: theme
            text: "Больше музыки"
            onClicked: {
                settingsStore.compactNavigation = true;
                settingsStore.sideQueueVisible = false;
                settingsStore.compactPlayer = true;
                settingsStore.homeHeroVisible = false;
            }
        }
    }
    Common.AuroraSwitch {
        uiTheme: theme
        Layout.fillWidth: true
        text: "Компактная навигация"
        subtext: "Иконки вместо широкой боковой панели"
        checked: settingsStore.compactNavigation
        onToggled: settingsStore.compactNavigation = checked
    }
    Common.AuroraSwitch {
        uiTheme: theme
        Layout.fillWidth: true
        text: "Очередь рядом"
        subtext: "Боковая панель на широком экране и при прослушивании"
        checked: settingsStore.sideQueueVisible
        onToggled: settingsStore.sideQueueVisible = checked
    }
    Common.AuroraSwitch {
        uiTheme: theme
        Layout.fillWidth: true
        text: "Компактный нижний плеер"
        subtext: "Меньше высота, больше места коллекции"
        checked: settingsStore.compactPlayer
        onToggled: settingsStore.compactPlayer = checked
    }
    Common.AuroraSwitch {
        uiTheme: theme
        Layout.fillWidth: true
        text: "Моя волна на главной"
        subtext: "Большая карточка с атмосферой и подборкой"
        checked: settingsStore.homeHeroVisible
        onToggled: settingsStore.homeHeroVisible = checked
    }
    ColorDialog {
        id: customColor
        title: "Цвет твоей Aurora"
        selectedColor: settingsStore.customAccent
        onAccepted: {
            settingsStore.customAccent = selectedColor.toString();
            settingsStore.colorScheme = "custom";
        }
    }
}
