import QtQuick 1.1
import org.kde.games.core 0.1 as KgCore

Item {
    id: canvas

    KgCore.CanvasItem {
        id: background
        spriteKey: "mainWidget"
        anchors.fill: parent
    }

    MineField {
        width: height
        height: Math.floor(parent.height/rows)*rows
        rows: 10
        columns: 10
        anchors.centerIn: parent
    }
}
