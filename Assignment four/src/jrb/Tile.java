package jrb;

/**
 *  Tile.java
 *  Holds a list of cells that compose a tile. NOTE: this is stored as a 1D array
 *  even though tiles are actually a 2D subset of the board.
 * @author Andrew Downie - 0786342
 */
public class Tile {
    //_the cells this tile is composed of
    Cell[] cells;
    //_The ID of this tile in the 1D array of tiles held by the board
    public int tileID;
    
    //_CONSTRUCTOR
    public Tile(Cell[] cellsContained, int tileID){
        cells = cellsContained;
        this.tileID = tileID;
    }
}
