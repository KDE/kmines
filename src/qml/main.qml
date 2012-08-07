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
import "logic.js" as Logic

Item {
    id: canvas

    signal minesCountChanged(int count, int total)
    signal firstClickDone
    signal gameOver(bool won)

    property bool useQuestionMarks: true

    function startNewGame(rows, cols, mines) {
        Logic.reset();
        field.rows = rows;
        field.columns = cols;
        field.mines = mines;
    }

    KgCore.CanvasItem {
        id: background
        spriteKey: "mainWidget"
        anchors.fill: parent
    }

    function setGamePaused(paused) {
        field.visible = !paused;
    }

    MineField {
        id: field
        anchors.centerIn: parent
        onCellClicked: Logic.revealCell(index);
        onMinesChanged: canvas.minesCountChanged(flaggedMines, mines);
        onFlaggedMinesChanged: canvas.minesCountChanged(flaggedMines, mines);
    }
}
