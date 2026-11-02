import QtQuick
import QtQuick.Controls
import "../common" as Common

ListView {
    id: root
    Theme { id: theme }
    model: libraryModel
    clip: true
    spacing: 2
    boundsBehavior: Flickable.StopAtBounds
    ScrollBar.vertical: Common.AuroraScrollBar { uiTheme: theme }
    delegate: TrackDelegate {
        onPlayRequested: row => player.playIndex(row)
        onRateRequested: (id, value) => libraryModel.setRating(id, value)
        onRemoveRequested: id => libraryModel.removeTrack(id)
    }
}
