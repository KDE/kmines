var firstClick = true;

function reset() {
    field.rows = 0;
    field.columns = 0;
    field.flaggedMines = 0;
    firstClick = true;
    canvas.game_over = false;
}

function revealNeighbours(index) {
    var item = field.itemAtIndex(index);
    var list = adjacentCells(index);
    var flaggedNeighbours = 0;
    for (var i=0; i<list.length; i++) {
        if (field.itemAtIndex(list[i]).flagged) flaggedNeighbours++;
    }
    if (flaggedNeighbours < item.digit) return;

    for (var i=0; i<list.length; i++) {
        var cell = field.itemAtIndex(list[i]);
        if (cell.flagged || cell.questioned) continue;
        revealCell(list[i]);
    }
}

function revealCell(index) {
    if (firstClick) {
        firstClick = false;
        generateField(index);
        canvas.firstClickDone();
    }
    var cell = field.itemAtIndex(index);
    cell.revealed = true;
    if (cell.hasMine) {
        cell.exploded = true;
        revealAllMines();
        canvas.gameOver(false);
    } else {
        if (cell.digit == 0)
            revealEmptyCells(index);
        checkWon();
    }
}

function revealAllMines() {
    for (var i=0; i<field.rows*field.columns; i++) {
        var item = field.itemAtIndex(i);
        if ( (item.flagged && !item.hasMine) || (!item.flagged && item.hasMine) ) {
            item.revealed = true;
        }
    }
}

function revealEmptyCells(index) {
    // recursively reveal neighbour cells until we find cells with digit
    var list = adjacentCells(index);
    var item;

    for (var i=0; i<list.length; i++) {
        // first is row, second is col
        item = field.itemAtIndex(list[i]);
        if (item.revealed || item.flagged || item.questioned)
            continue;
        if (item.digit == 0) {
            item.revealed = true;
            revealEmptyCells(list[i]);
        } else {
            item.revealed = true;
        }
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
        cell = field.itemAtIndex(randomIndex);
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
            cell = field.itemAtIndex(neighbours[j]);
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

function checkWon() {
    // this also takes into account the trivial case when
    // only some cells left unflagged and they
    // all contain bombs. this counts as win
    if(field.unrevealedMines == field.mines)
    {
        // mark not flagged cells (if any) with flags
        for (var i=0; i<field.rows*field.columns; i++) {
            var item = field.itemAtIndex(i);
            if( !item.revealed && !item.flagged )
                item.flagged = true;
        }
        // now all mines should be flagged, notify about this
        canvas.minesCountChanged(field.unrevealedMines, field.mines);
        canvas.gameOver(true);
    }
}
