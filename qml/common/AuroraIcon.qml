// Векторная иконка: рисует SVG-путь по имени, с поддержкой цвета и размера.
import QtQuick
import QtQuick.Shapes
import ".."

Item {
    id: root
    property Theme uiTheme
    // Имя иконки из словаря paths ниже.
    property string name: ""
    property color color: uiTheme ? uiTheme.text : "#eaf2ff"
    property real size: 20

    implicitWidth: size
    implicitHeight: size

    // Словарь SVG-путей иконок, адресуемых по имени.
    readonly property var paths: ({
            "clock": "M12 3a9 9 0 1 0 0 18 9 9 0 0 0 0-18M12 7v5l3 2",
            "equalizer": "M5 3v18M12 3v18M19 3v18M2 8h6M9 16h6M16 10h6",
            "muted": "M3 9h4l5-4v14l-5-4H3zM16 9l6 6M22 9l-6 6",
            "heart": "M12 20S3 14 3 8.5C3 3 10 2 12 7c2-5 9-4 9 1.5C21 14 12 20 12 20z",
            "shuffle": "M3 6h3c4 0 8 12 12 12h3M17 14l4 4-4 4M3 18h3c2 0 3-2 4-4M14 9c1-2 2-3 4-3h3M17 2l4 4-4 4",
            "repeat": "M4 8a3 3 0 0 1 3-3h13M17 2l3 3-3 3M20 16a3 3 0 0 1-3 3H4M7 16l-3 3 3 3",
            "queue": "M3 5h18M3 11h11M3 17h8M17 13l5 4-5 4z",
            "expand": "M8 3H3v5M16 3h5v5M3 16v5h5M21 16v5h-5",
            "volume": "M3 9h4l5-4v14l-5-4H3zM16 8c3 2 3 6 0 8M19 5c5 4 5 10 0 14",
            "more": "M5 12h.01M12 12h.01M19 12h.01",
            "home": "M3 10l9-7 9 7M5 9v12h5v-7h4v7h5V9",
            "play": "M8 5l11 7-11 7z",
            "pause": "M8 5v14M16 5v14",
            "next": "M5 5l10 7-10 7zM18.5 5v14",
            "previous": "M19 5l-10 7 10 7zM5.5 5v14",
            "download": "M12 4v11M7 11l5 5 5-5M5 20h14",
            "upload": "M12 16V5M7 9l5-5 5 5M5 20h14",
            "folder": "M3 6a2 2 0 0 1 2-2h5l2 3h7a2 2 0 0 1 2 2v9a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z",
            "search": "M10.5 4a6.5 6.5 0 1 0 0 13 6.5 6.5 0 0 0 0-13zM15.5 15.5L21 21",
            "settings": "M4 7h9M17 7h3M4 17h3M11 17h9M15 4v6M9 14v6",
            "library": "M4 4v16M9 4v16M14 5l5-1 3 15-5 1z",
            "plus": "M12 5v14M5 12h14",
            "close": "M6 6l12 12M18 6L6 18",
            "check": "M5 12.5l4.5 4.5L19 7.5",
            "moon": "M20 15A8.5 8.5 0 0 1 9 4a8.5 8.5 0 1 0 11 11z",
            "chevron": "M9 5l7 7-7 7",
            "music": "M9 18V6l10-2v12M9 9l10-2M5.5 15A3.5 3.5 0 1 0 5.5 22 3.5 3.5 0 1 0 5.5 15M15.5 13A3.5 3.5 0 1 0 15.5 20 3.5 3.5 0 1 0 15.5 13",
            "link": "M10 14a4 4 0 0 0 5.7 0l3-3a4 4 0 0 0-5.7-5.7l-1 1M14 10a4 4 0 0 0-5.7 0l-3 3a4 4 0 0 0 5.7 5.7l1-1",
            "trash": "M4 7h16M9 7V4h6v3M6 7l1 13h10l1-13M10 11v5M14 11v5",
            "refresh": "M20 12a8 8 0 1 1-2.34-5.66M20 4v5h-5"
        })

    Shape {
        // Иконка рисуется в сетке 24x24 и масштабируется под нужный размер.
        width: 24
        height: 24
        anchors.centerIn: parent
        scale: root.size / 24
        preferredRendererType: Shape.CurveRenderer

        ShapePath {
            strokeColor: root.color
            strokeWidth: 2
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            PathSvg {
                // Если иконка не найдена — рисуем пустой путь.
                path: root.paths[root.name] || ""
            }
        }
    }
}
