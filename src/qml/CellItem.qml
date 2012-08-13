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

KgCore.CanvasItem {
    id: cell
    spriteKey: border!="" ? border : exploded ? "explosion" : (pressed || revealed ? "cell_down" : "cell_up")

    property string border

    property bool hasMine: false
    property int digit: 0

    property bool pressed: false
    property bool revealed: false
    property bool exploded: false
    property bool flagged: cellState == 1
    property bool questioned: cellState == 2
    property int cellState: 0
    property bool error: !hasMine && flagged

    signal clicked
    signal doubleClicked

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        enabled: (!revealed || digit>0) && border==""
        onPressed: {
            if (canvas.game_over) return;
            if (mouse.button == Qt.LeftButton && !flagged && !questioned) {
                cell.pressed = true;
            }
        }
        onReleased: {
            if (canvas.game_over) return;
            cell.pressed = false;
            if (mouse.button == Qt.LeftButton) {
                if (flagged || questioned) return;
                cell.clicked();
            } else if (!revealed) {
                if (flagged && !canvas.useQuestionMarks)
                    cellState = (cellState+2)%3;
                else
                    cellState = (cellState+1)%3;
            }
        }
        onDoubleClicked: {
            if (digit==0 || flagged || questioned) return;
            cell.doubleClicked();
        }
    }

    KgCore.CanvasItem {
        anchors.fill: parent
        visible: (parent.hasMine||flagged) && revealed
        spriteKey: "mine"
    }

    KgCore.CanvasItem {
        anchors.fill: parent
        spriteKey: revealed && error ? "error" : ["", "flag", "question"][cellState]
        visible: cellState>0
    }

    KgCore.CanvasItem {
        anchors.fill: parent
        visible: digit>0 && revealed && !error
        spriteKey: "arabic" + ["One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight"][digit-1]
    }
}
