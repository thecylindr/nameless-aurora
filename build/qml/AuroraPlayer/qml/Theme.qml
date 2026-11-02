import QtQuick

QtObject {
    readonly property color base: "#05070f"
    readonly property color surface: "#121626"
    readonly property color surfaceHigh: "#1b2040"
    readonly property color accent: "#0a84ff"
    readonly property color accent2: "#5e5ce6"
    readonly property color text: "#eaf2ff"
    readonly property color subtext: "#9aa4c8"
    readonly property color line: "#1effffff"
    readonly property color good: "#34c759"
    readonly property color warn: "#ffd60a"
    readonly property color danger: "#ff455a"

    readonly property color background: base
    readonly property color sidebar: base
    readonly property color textStrong: text
    readonly property color muted: subtext
    readonly property color textMuted: "#6f7a9e"
    readonly property color outline: line
    readonly property color accentBright: accent2
    readonly property color blue: accent
    readonly property color hover: surfaceHigh
    readonly property color selected: "#260a84ff"

    readonly property int spacingSmall: 8
    readonly property int spacing: 12
    readonly property int spacingMedium: 16
    readonly property int spacingLarge: 24
    readonly property int radiusSmall: 10
    readonly property int radius: 16
    readonly property int radiusLarge: 24
    readonly property int controlHeight: 44
    readonly property int touchHeight: 48
    readonly property int smallSize: 10
    readonly property int captionSize: 11
    readonly property int bodySize: 13
    readonly property int titleSize: 20
    readonly property int headingSize: 28
}
