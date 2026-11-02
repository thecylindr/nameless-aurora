import QtQuick
import QtQuick.Shapes
import ".."

Item {
    id: root
    property Theme uiTheme
    property string name: ""
    property color color: uiTheme ? uiTheme.text : "#eaf2ff"
    property real size: 20

    implicitWidth: size
    implicitHeight: size

    readonly property var paths: ({
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
        "chevron": "M9 5l7 7-7 7",
        "music": "M9 18V6l10-2v12M9 9l10-2M5.5 15A3.5 3.5 0 1 0 5.5 22 3.5 3.5 0 1 0 5.5 15M15.5 13A3.5 3.5 0 1 0 15.5 20 3.5 3.5 0 1 0 15.5 13",
        "link": "M10 14a4 4 0 0 0 5.7 0l3-3a4 4 0 0 0-5.7-5.7l-1 1M14 10a4 4 0 0 0-5.7 0l-3 3a4 4 0 0 0 5.7 5.7l1-1",
        "trash": "M4 7h16M9 7V4h6v3M6 7l1 13h10l1-13M10 11v5M14 11v5",
        "refresh": "M20 12a8 8 0 1 1-2.34-5.66M20 4v5h-5"
    })

    Shape {
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
            PathSvg { path: root.paths[root.name] || "" }
        }
    }
}
