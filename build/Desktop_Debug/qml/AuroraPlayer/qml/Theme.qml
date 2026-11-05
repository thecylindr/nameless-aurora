// Центральный объект темы: палитры, размеры, отступы и производные цвета для всего приложения.
import QtQuick

QtObject {
    // Движение/анимации включены только когда приложение активно.
    readonly property bool motionEnabled: settingsStore.motionEnabled && Application.state !== Qt.ApplicationSuspended && Application.state !== Qt.ApplicationHidden
    readonly property string scheme: settingsStore.colorScheme
    // Набор встроенных цветовых схем.
    readonly property var palettes: ({
            aurora: {
                base: "#0d0f17",
                surface: "#171a25",
                high: "#232735",
                accent: "#baa5ff",
                second: "#83dbc7"
            },
            lagoon: {
                base: "#0a1418",
                surface: "#121f25",
                high: "#20323a",
                accent: "#7fdfcb",
                second: "#7aaff3"
            },
            ember: {
                base: "#17100f",
                surface: "#241a19",
                high: "#362725",
                accent: "#fbb38d",
                second: "#e2a2be"
            },
            rose: {
                base: "#160e18",
                surface: "#241a29",
                high: "#34273b",
                accent: "#eba8cc",
                second: "#b2acff"
            },
            midnight: {
                base: "#0a101b",
                surface: "#131d2d",
                high: "#203049",
                accent: "#8ab8ff",
                second: "#afa6ff"
            }
        })
    // Активная палитра; при неизвестной схеме откатываемся к «aurora».
    readonly property var palette: palettes[scheme] || palettes.aurora
    readonly property color base: palette.base
    readonly property color surface: palette.surface
    readonly property color surfaceHigh: palette.high
    // Акцентный цвет может приходить извне (nameless) или настраиваться пользователем.
    readonly property color accent: scheme === "nameless" ? namelessBridge.accent : scheme === "custom" ? settingsStore.customAccent : palette.accent
    readonly property color accent2: scheme === "nameless" ? namelessBridge.secondary : palette.second
    readonly property color text: "#f3f1f8"
    readonly property color subtext: "#a4a5b7"
    // Линия-разделитель: акцент с низкой прозрачностью.
    readonly property color line: Qt.rgba(accent.r, accent.g, accent.b, 0.09)
    readonly property color good: "#8bdbb0"
    readonly property color warn: "#f3ca85"
    readonly property color danger: "#ff8b9f"
    readonly property color background: base
    readonly property color sidebar: Qt.darker(base, 1.12)
    // Цвет текста поверх акцента выбирается по яркости (светлый/тёмный).
    readonly property color textStrong: (accent.r * 0.2126 + accent.g * 0.7152 + accent.b * 0.0722) < 0.5 ? "#f3f1f8" : "#12131a"
    readonly property color muted: subtext
    readonly property color textMuted: "#7c7e92"
    readonly property color outline: line
    readonly property color accentBright: accent
    readonly property color blue: accent
    // Цвет наведения — слегка осветлённая поверхность.
    readonly property color hover: Qt.lighter(surfaceHigh, 1.12)
    // Полупрозрачный фон выбранного элемента.
    readonly property color selected: Qt.rgba(accent.r, accent.g, accent.b, 0.13)
    readonly property int spacingSmall: 8
    readonly property int spacing: 12
    readonly property int spacingMedium: 16
    readonly property int spacingLarge: 24
    readonly property int radiusSmall: 10
    readonly property int radius: 16
    readonly property int radiusLarge: 24
    readonly property int controlHeight: 46
    readonly property int touchHeight: 44
    readonly property int smallSize: 10
    readonly property int captionSize: 12
    readonly property int bodySize: 14
    readonly property int titleSize: 22
    readonly property int headingSize: 32
}
