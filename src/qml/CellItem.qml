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

    property bool hasMine: false
    property int digit: 0

    signal clicked

    MouseArea {
        anchors.fill: parent
        enabled: spriteKey=="cell_up" || spriteKey=="cell_down"
        onPressed: {
            if (spriteKey == "cell_up") {
                spriteKey = "cell_down";
            }
        }
        onReleased: {
            if (spriteKey == "cell_down") {
                spriteKey = "cell_up";
                cell.clicked();
            }
        }
    }
}
