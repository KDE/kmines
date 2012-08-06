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

    signal cellClicked(int index)

    property int rows
    property int columns
    property int mines

    property alias cells: cellRepeater

    Grid {
        id: field
        anchors.fill: parent
        rows: parent.rows+2
        columns: parent.columns+2

        Repeater {
            id: cellRepeater
            model: (rows+2)*(columns+2)

            CellItem {
                width: field.width/field.rows
                height: field.height/field.columns

                property int row: Math.floor(index/field.rows)
                property int column: index%field.columns

                spriteKey: getKeyFromPos(row, column)

                onClicked: container.cellClicked(index);
            }
        }
    }

    function getKeyFromPos(row, col) {
        if( row == 0 && col == 0)
        {
            return "border.outsideCorner.nw";
        }
        else if( row == 0 && col == columns+1)
        {
            return "border.outsideCorner.ne";
        }
        else if( row == rows+1 && col == 0 )
        {
            return "border.outsideCorner.sw";
        }
        else if( row == rows+1 && col == columns+1 )
        {
            return "border.outsideCorner.se";
        }
        else if( row == 0 )
        {
            return "border.edge.north";
        }
        else if( row == rows+1 )
        {
            return "border.edge.south";
        }
        else if( col == 0 )
        {
            return "border.edge.west";
        }
        else if( col == columns+1 )
        {
            return "border.edge.east";
        }
        else {
            return "cell_up";
        }
    }
}
