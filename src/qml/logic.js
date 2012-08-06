var firstClick = true;

function revealCell(index) {
    if (firstClick) {
        firstClick = false;
        generateField(index);
    }
}

function generateField(clickedIndex) {
    // generating mines ensuring that clickedIdx won't hold mine
    // and that it will be an empty cell so the user don't have
    // to make random guesses at the start of the game
    var cellsWithMines = [];
    var minesToPlace = field.mines;
    var randomIndex;
    var cell;

    // this is the list of items we don't want to put the mine in
    // to ensure that clickedIdx will stay an empty cell
    // (it will be empty if none of surrounding items holds mine)
    var adjacentIndex = adjacentCells(clickedIndex);
    
    while (minesToPlace != 0) {
        randomIndex = Math.floor(Math.random()*field.rows*field.columns);
        cell = field.cells.itemAt(randomIndex);
        if (!cell.hasMine
            && adjacentIndex.indexOf(randomIndex) == -1
            && randomIndex != clickedIndex) {
            // ok, let's mine this place! :-)
            cell.hasMine = true;
            cellsWithMines.push(randomIndex);
            minesToPlace--;
        }
    }

    for (var i=0; i<cellsWithMines.length; i++) {
        var neighbours = adjacentCells(cellsWithMines[i]);
        for (var j=0; j<neighbours.length; j++) {
            cell = field.cells.itemAt(neighbours[j]);
            if (!cell.hasMine)
                cell.digit++;
        }
    }
}

function adjacentCells(index) {
    var row = Math.floor(index/field.columns);
    var col = index%field.columns;
    var adjacent = [];

    if(row != 0 && col != 0) // upper-left diagonal
        adjacent.push(index - field.columns - 1);
    if(row != 0) // upper
        adjacent.push(index - field.columns);
    if(row != 0 && col != field.columns-1) // upper-right diagonal
        adjacent.push(index - field.columns + 1);
    if(col != 0) // on the left
        adjacent.push(index - 1);
    if(col != field.columns-1) // on the right
        adjacent.push(index + 1);
    if(row != field.rows-1 && col != 0) // bottom-left diagonal
        adjacent.push(index + field.columns - 1);
    if(row != field.rows-1) // bottom
        adjacent.push(index + field.columns);
    if(row != field.rows-1 && col != field.columns-1) // bottom-right diagonal
        adjacent.push(index + field.columns + 1);

    return adjacent;
}
