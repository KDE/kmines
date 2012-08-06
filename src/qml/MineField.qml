/*
    Copyright 2012 Viranch Mehta <viranch.mehta@gmail.com>
  
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.
   
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
   
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

import QtQuick 1.1
import org.kde.games.core 0.1 as KgCore

Item {
    id: container

    width: height*(columns+2)/(rows+2)
    height: Math.floor(parent.height/(rows+2))*(rows+2)

    signal cellClicked(int index)

    property int rows
    property int columns
    property int mines

    property alias cells: cellRepeater

    property real cellSize: width/(columns+2)

    /* === BORDERS === */

    KgCore.CanvasItem {
        spriteKey: "border.outsideCorner.nw"
        anchors { top: parent.top; left: parent.left; bottom: field.top; right: field.left }
    }
    KgCore.CanvasItem {
        spriteKey: "border.outsideCorner.ne"
        anchors { top: parent.top; left: field.right; bottom: field.top; right: parent.right }
    }
    KgCore.CanvasItem {
        spriteKey: "border.outsideCorner.sw"
        anchors { top: field.bottom; left: parent.left; bottom: parent.bottom; right: field.left }
    }
    KgCore.CanvasItem {
        spriteKey: "border.outsideCorner.se"
        anchors { top: field.bottom; left: field.right; bottom: parent.bottom; right: parent.right }
    }

    Row {
        anchors {
            top: parent.top
            left: field.left
            right: field.right
            bottom: field.top
        }
        Repeater {
            model: columns
            KgCore.CanvasItem {
                spriteKey: "border.edge.north"
                width: cellSize
                height: cellSize
            }
        }
    }

    Column {
        anchors {
            top: field.top
            left: parent.left
            right: field.left
            bottom: field.bottom
        }
        Repeater {
            model: rows
            KgCore.CanvasItem {
                spriteKey: "border.edge.west"
                width: cellSize
                height: cellSize
            }
        }
    }

    Column {
        anchors {
            top: field.top
            left: field.right
            right: parent.right
            bottom: field.bottom
        }
        Repeater {
            model: rows
            KgCore.CanvasItem {
                spriteKey: "border.edge.east"
                width: cellSize
                height: cellSize
            }
        }
    }

    Row {
        anchors {
            top: field.bottom
            left: field.left
            right: field.right
            bottom: parent.bottom
        }
        Repeater {
            model: columns
            KgCore.CanvasItem {
                spriteKey: "border.edge.south"
                width: cellSize
                height: cellSize
            }
        }
    }

    /* === END BORDERS === */

    Grid {
        id: field
        anchors {
            fill: parent
            margins: cellSize
        }
        rows: parent.rows
        columns: parent.columns

        Repeater {
            id: cellRepeater
            model: (rows+0)*(columns+0)

            CellItem {
                width: field.width/field.columns
                height: field.height/field.rows

                property int row: Math.floor(index/field.rows)
                property int column: index%field.columns

                spriteKey: "cell_up"

                onClicked: container.cellClicked(index);
            }
        }
    }
}
