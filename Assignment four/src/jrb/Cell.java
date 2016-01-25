package jrb;//THis is non exp

/**
 *  Cell.java
 *  This class represents a single cell, in the board. It holds a color, as well
 *  as well as references to its neighbors, as well as whether or not this cell
 *  has been touched already for the current step.
 * @author Andrew Downie - 0786342
 */
public class Cell {
    //_The current color of this cell
    private CellContents contents;
    //_The neighbor cells that will need to be checked during the simulation
    private Cell right, down;
    
    //_Used to make sure the board is moved in the correct order (COULD THIS BE THE PROBLEM FOR INACCURATE RESULTS ON BIG BOARDS)
    public boolean redFrom, redTo;
    public boolean blueFrom, blueTo;
    
    //_CONSTRUCTOR
    public Cell(int zeroOneTwo){
        if(zeroOneTwo == 0){
            contents = CellContents.White;
        }
        else if (zeroOneTwo == 1){
            contents = CellContents.Red;
        }
        else if(zeroOneTwo == 2){
            
            contents = CellContents.Blue;
            
        }
        else{
            System.out.println("Cell recieved number that was not one, two or three in constructor.");
        }
        
    }
    
    //_This is slow and should be replaced
    public void Reset(){
        redFrom = false;
        redTo = false;
        blueFrom = false;
        blueTo = false;
    }
    public void setBlueNeighbour(Cell cell){
        right = cell;
    }
    public void setRedNeighbour(Cell cell){
        down = cell;
    }
    public Cell blueNeighbour(){
        return right;
    }
    public Cell redNeighbour(){
        return down;
    }
    public void SetContents(CellContents contents){
        this.contents = contents;
    }
    public CellContents contents(){
        return contents;
    }
    
    //_Debugging stuff
  
}
