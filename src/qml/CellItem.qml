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
    spriteKey: revealed ? "cell_down" : "cell_up"

    property bool hasMine: false
    property int digit: 0

    property bool revealed: false

    signal clicked

    MouseArea {
        anchors.fill: parent
        enabled: spriteKey=="cell_up" || spriteKey=="cell_down"
        onPressed: {
            revealed = true;
        }
        onReleased: {
            cell.clicked();
        }
    }

    KgCore.CanvasItem {
        anchors.fill: parent
        visible: parent.hasMine && revealed
        spriteKey: "mine"
    }

    KgCore.CanvasItem {
        anchors.fill: parent
        visible: digit>0 && revealed
        spriteKey: "arabic" + ["One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight"][digit-1]
    }
}
