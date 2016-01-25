package jrb;//TODO:::: GET THE SetupTile() function working properly


import java.io.PrintWriter;
import java.util.Random;

/**
 *  Board.java
 *  Holds a 2D array of cells. Generates the CellContents randomly based on a 
 *  seed passed in during initialization. Also generates an array of the tiles
 *  for the board, as well as links each cell with its right and bottom cell
 *  neighbor.
 * @author Andrew Downie - 0786342
 */
public class Board {
    //The 2D array of cells
    private Cell[][] cells;
    //_The list of tiles that cover the board
    private Tile[] tiles;
    //_The boardSize from the command line arguments
    private int boardSize;
    //_The tileSize from the command line arguments
    private int tileSize;
    //_The seed used to generate the board
    private long seed;
    
    //_Global flag that ends the simulation if the color density was met
    public boolean densityMet;
    private int colorCount = 0;
    public int steps = -1;
    
    public synchronized void SetColorCount(int newDensity){
        if(newDensity > colorCount){
            colorCount = newDensity;
        }
    }
    
    public void ResetColorCount(){
        colorCount = 0;
    }
    public int colorCount(){
        return colorCount;
    }

    //_CONSTRUCTOR
    public Board(int boardSize, int tileSize, long boardSeed) {
        this.boardSize = boardSize;
        this.tileSize = tileSize;
        seed = boardSeed;

        cells = new Cell[boardSize][boardSize];
        RandomizeBoard();

        tiles = new Tile[(boardSize * boardSize) / (tileSize * tileSize)];
        SetupTiles();

        SetCellNeighbours();
    }

    //_Fills in the list of tile
    private void SetupTiles() {
        Cell[] tileCells;
        int cellsAdded, tilesAdded = 0;

        //_Go through the top left cell of each tile
        for (int x = 0; x < boardSize; x += tileSize) {
            for (int y = 0; y < boardSize; y += tileSize) {
                tileCells = new Cell[tileSize * tileSize];
                cellsAdded = 0;

                //_Go through all the cells in a tile, starting from the 
                //_top left of the tile were currently looking at
                for (int x1 = x; x1 < tileSize + x; x1++) {
                    for (int y1 = y; y1 < tileSize + y; y1++) {
                        tileCells[cellsAdded] = cells[x1][y1];
                        cellsAdded++;
                    }
                }
                tiles[tilesAdded] = new Tile(tileCells, tilesAdded);
                tilesAdded++;
            }
        }
    }

    //_Sets the neighbors of the cells, requires the cells are already setup
    private void SetCellNeighbours() {
        for (int x = 0; x < boardSize; x++) {
            for (int y = 0; y < boardSize; y++) {
                cells[x][y].setRedNeighbour(cells[x][WrapIndex(y + 1)]);
                cells[x][y].setBlueNeighbour(cells[WrapIndex(x + 1)][y]);
            }
        }
    }

    //_Loop from one edge of the board to the other
    private int WrapIndex(int index) {
        if (index >= boardSize) {
            return index - boardSize;
        }
        else if (index < 0){
            return boardSize - 1;//IS THIS CORRECT??????????... looks like it
        }
        
        return index;
    }

    //_Fill in the cells with random colors
    private void RandomizeBoard() {
        Random rnd = new Random(seed);

        for (int x = 0; x < boardSize; x++) {
            for (int y = 0; y < boardSize; y++) {
                cells[x][y] = new Cell(rnd.nextInt(3));
            }
        }
    }

    //Getters
    public int cellsLength() {
        return boardSize * boardSize;
    }
    public int tilesLength(){
        return tiles.length;
    }

    public int boardSize() {
        return boardSize;
    }

    public Cell cell(int x, int y) {
        return cells[x][y];
    }
    
    public Tile tile(int index){
        return tiles[index];
    }
    
     /*public synchronized void MoveRedCells(Cell[] myRedCells) {
     
        Cell curCell;

        
        for (int i = 0; i < myRedCells.length; i++) {
            curCell = myRedCells[i];
            if (curCell.contents() != CellContents.Red || curCell.redTo || curCell.redFrom || curCell.redNeighbour().contents() != CellContents.White
                    || curCell.redNeighbour().redFrom || curCell.redNeighbour().redTo) {
                continue;
            }
            
            curCell.redFrom = true;
            curCell.redNeighbour().redTo = true;

            
            curCell.SetContents(CellContents.White);
            curCell.redNeighbour().SetContents(CellContents.Red);
           
        }

        

    }*/
     
    /* public synchronized void MoveBlueCells(Cell[] myBlueCells) {
        Cell curCell;

      
        for (int i = 0; i < myBlueCells.length; i++) {
            curCell = myBlueCells[i];
            if (curCell.contents() != CellContents.Blue || curCell.blueTo || curCell.blueFrom || curCell.blueNeighbour().contents() != CellContents.White
                    || curCell.blueNeighbour().blueFrom || curCell.blueNeighbour().blueTo) {

                continue;
            }
            
            curCell.blueFrom = true;
            curCell.blueNeighbour().blueTo = true;
            
            curCell.SetContents(CellContents.White);
            curCell.blueNeighbour().SetContents(CellContents.Blue);
           

        }

        
    }*/
     
    

    //_Debugging Stuff
    public void WriteBoard(PrintWriter pw) {
        //String output = "";
        //String curRow;

        //System.out.println("Printing board...");
        for (int x = 0; x < boardSize; x++) {
            //curRow = "";
            for (int y = 0; y < boardSize; y++) {
                pw.print(StringCell(cells[x][y]));
                pw.flush();
                //curRow += StringCell(cells[x][y]);

            }
            pw.print("\n");
            //System.out.println(curRow);
            //output += curRow + "\n";
        }
        
        //return output;
    }
    
    public void PrintBoard(){
     String curRow;

        //System.out.println("Printing board...");
        for (int x = 0; x < boardSize; x++) {
            curRow = "";
            for (int y = 0; y < boardSize; y++) {
                curRow += StringCell(cells[x][y]);

            }
            System.out.println(curRow);
        }
    }

    public void PrintTiles() {
        for (int i = 0; i < tiles.length; i++) {
            PrintTile(i);
        }
    }

    public void PrintTile(int tileIndex) {
        if (tileIndex >= tiles.length || tileIndex < 0) {
            System.out.println("PrintTile given index out of range");
            return;
        }

        String line;
        int curCell = 0;
        System.out.println("Tile " + tileIndex);
        for (int x = 0; x < tileSize; x++) {
            line = "\t";
            for (int y = 0; y < tileSize; y++) {
                line += StringCell(tiles[tileIndex].cells[curCell]);
                curCell++;
            }

            System.out.println(line);
        }
    }

    public void PrintCellNeighbours() {
        for (int x = 0; x < boardSize; x++) {
            for (int y = 0; y < boardSize; y++) {
                PrintCellNeighbour(x, y);
            }
        }
    }

    public void PrintCellNeighbour(int x, int y) {
        System.out.println("Cell: " + x + "," + y + " has neighbours:");
        System.out.println("\t" + StringCell(cells[x][y]) + StringCell(cells[x][y].blueNeighbour()));
        System.out.println("\t" + StringCell(cells[x][y].redNeighbour()));
    }
    
    private String StringCell(Cell cell){
        CellContents c = cell.contents();
        
        if(c == CellContents.Blue){
            return "V";
        }
        else if (c == CellContents.Red){
            return ">";
        }
        
        return " ";
    }
}
