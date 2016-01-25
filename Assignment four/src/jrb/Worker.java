package jrb;

import java.io.IOException;
import java.util.concurrent.BrokenBarrierException;
import java.util.concurrent.CyclicBarrier;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * Worker.java
 * This class is responsible for the actual simulation of the red/blue problem.
 * Upon initialization this class grabs all cells it will be responsible to move
 * each cycle. It also grabs all tiles it is responsible for checking between
 * steps.
 * @author Andrew Downie - 0786342
 */
public class Worker implements Runnable {
    //_Cells this worker will move on the red section of a stage
    private Cell[] myBlueCells;
    //_Cells this worker will move on the blue section of a stage
    private Cell[] myRedCells;
    //_Tiles this worker will check for density on the tile section of a stage
    private Tile[] myTiles;

    //_Reference to the barrier used to sync all workers
    private CyclicBarrier barrier;

    //_This workers ID, unique value
    private int workerID;
    //_Reference to the board, shared throughout the program
    private Board board;

    //From command line arguments
    private int maxColorDensity;
    private int currentStep = 1;
    private int workerCount;
    private int maxSteps;

    //_CONSTRUCTOR
    public Worker(int workerID, CmdLineArgs cla, Board board, CyclicBarrier barrier) {
        this.maxColorDensity = cla.maxColorDensity();
        this.workerCount = cla.workerCount();
        this.maxSteps = cla.maxSteps();
        maxColorDensity = (cla.tileSize() * cla.tileSize() * cla.maxColorDensity()) / 100;

        this.workerID = workerID;
        this.barrier = barrier;
        this.board = board;

        GetMyRedCells();
        GetMyBlueCells();
        GetMyTiles();
    }

    //_Grabs a list of cells to move during the blue stage from the board,
    //_These cells are unique to this worker during the blue section
    private void GetMyBlueCells() {//Blue tiles will become my new
        int blueCellCount;
        int curIndex = 0;
        int div, rem;

        blueCellCount = board.cellsLength() / workerCount;
        if ((workerCount - workerID - 1) < (board.cellsLength() % workerCount)) {
            blueCellCount++;
        }

        myBlueCells = new Cell[blueCellCount];

        for (int i = (workerCount - workerID - 1); i < board.cellsLength(); i += workerCount) {
            div = i / board.boardSize();
            rem = i % board.boardSize();

            myBlueCells[curIndex] = board.cell(div, rem);
            curIndex++;
        }
    }

    //_Grabs a list of cells to move during the red stage from the board,
    //_These cells are unique to this worker during the blue section
    private void GetMyRedCells() {
        int redCellCount;
        int curIndex = 0;
        int div, rem;

        redCellCount = board.cellsLength() / workerCount;
        if (workerID < board.cellsLength() % workerCount) {
            redCellCount++;
        }

        myRedCells = new Cell[redCellCount];

        for (int i = workerID; i < board.cellsLength(); i += workerCount) {
            div = i / board.boardSize();
            rem = i % board.boardSize();
            //System.out.println("Div = " + div + ", Rem = " + rem);
            myRedCells[curIndex] = board.cell(div, rem);
            curIndex++;
        }
    }

    //_Gets the tiles that this worker should check at the end of every stage
    private void GetMyTiles() {
        int myTileCount, tileIndex = 0;

        myTileCount = board.tilesLength() / workerCount;
        if (workerID < board.tilesLength() % workerCount) {
            myTileCount++;
        }

        myTiles = new Tile[myTileCount];

        for (int i = workerID; i < board.tilesLength(); i += workerCount) {
            myTiles[tileIndex] = board.tile(i);
            tileIndex++;
        }
    }

    //_This is a bad way of doing this, and should go away
    private void ResetCells() {
        for (int i = 0; i < myRedCells.length; i++) {
            myRedCells[i].Reset();
        }

        //_No need to do this for blue cells, they're already covered
    }

    //_Moves the red cells (START THE DEBUGGING BY SIMPLIFYING THESE)
    private void MoveRedCells() {
     
        Cell curCell;

        
        for (int i = 0; i < myRedCells.length; i++) {
            curCell = myRedCells[i];
            if (curCell.contents() != CellContents.Red || curCell.redTo || curCell.redFrom || curCell.redNeighbour().contents() != CellContents.White
                    || curCell.redNeighbour().redFrom || curCell.redNeighbour().redTo) {
                continue;
            }
            
            curCell.redFrom = true;
            curCell.redNeighbour().redTo = true;
            /*curCell.SetContents(CellContents.White);
            curCell.redNeighbour().SetContents(CellContents.Red);*/
           
        }

        

    }

    //_Moves the blue cells
    private void MoveBlueCells() {
        Cell curCell;

      
        for (int i = 0; i < myBlueCells.length; i++) {
            curCell = myBlueCells[i];
            if (curCell.contents() != CellContents.Blue || curCell.blueTo || curCell.blueFrom || curCell.blueNeighbour().contents() != CellContents.White
                    || curCell.blueNeighbour().blueFrom || curCell.blueNeighbour().blueTo) {

                continue;
            }
            
            curCell.blueFrom = true;
            curCell.blueNeighbour().blueTo = true;
            /*curCell.SetContents(CellContents.White);
            curCell.blueNeighbour().SetContents(CellContents.Blue);*/
           

        }

        
    }
    
    //_Checks the tile that belong to this worker, if the color density condition
    //_has been met
    private void CheckTiles() {
        int blueCount = 0, redCount = 0;
        Cell[] currentTile;

        for (int tile = 0; tile < myTiles.length; tile++) {
            currentTile = myTiles[tile].cells;
            for (int cell = 0; cell < currentTile.length; cell++) {
                if (currentTile[cell].contents() == CellContents.Blue) {
                    blueCount++;
                }
                if (currentTile[cell].contents() == CellContents.Red) {
                    redCount++;
                }
            }
            
            if(redCount > blueCount){
                board.SetColorCount(redCount);
            }else{
                board.SetColorCount(blueCount);
            }

            if (redCount > maxColorDensity) {
               // System.out.println("Red density of: " + redCount + " met by tile: " + myTiles[tile].tileID + " in step: " + currentStep);
                board.densityMet = true;
                board.SetColorCount(redCount);
                board.steps = currentStep;
            }
            if (blueCount > maxColorDensity) {
                //System.out.println("Blue density of: " + blueCount + " met by tile: " + myTiles[tile].tileID + " in step: " + currentStep);
                board.densityMet = true;
                board.SetColorCount(blueCount);
                board.steps = currentStep;
            }

            blueCount = 0;
            redCount = 0;
        }
    }
    
    private void UpdateReds(Cell[] cells){
        for(int i = 0; i < cells.length; i++){
            if(cells[i].redFrom){
                cells[i].SetContents(CellContents.White);
                continue;
            }
            
            if(cells[i].redTo){
                cells[i].SetContents(CellContents.Red);
                continue;
            }
            
        }
    }
    
    private void UpdateBlues(Cell[] cells){
        for(int i = 0; i < cells.length; i++){
            if(cells[i].blueFrom){
                cells[i].SetContents(CellContents.White);
                continue;
            }
            
            if(cells[i].blueTo){
                cells[i].SetContents(CellContents.Blue);
                continue;
            }
            
        }
    }

    //_Runs this worker
    @Override
    public void run() {
        while (!board.densityMet && currentStep < maxSteps) {
            //Barrier();//For testing
            ResetCells();
            board.ResetColorCount();
            Barrier();

            
            MoveRedCells();
            Barrier();
            UpdateReds(myRedCells);
            Barrier();
            
            
            MoveBlueCells();
            Barrier();
            UpdateBlues(myBlueCells);
            Barrier();
           
            CheckTiles();
            //Prompt_Wait();
            
            Barrier();
            currentStep++;
            //Barrier();//For testing

        }
         //Barrier();//For testing
        currentStep--;
        CheckTiles();
         //Barrier();//For testing
    }

    //_A barrier for the workers
    private void Barrier() {
        try {
            barrier.await();
        } catch (InterruptedException ex) {
            Logger.getLogger(Worker.class.getName()).log(Level.SEVERE, null, ex);
        } catch (BrokenBarrierException ex) {
            Logger.getLogger(Worker.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    //Debugging stuff
    private void Prompt_Wait() {
        Barrier();
        if (workerID == 0) {
            board.PrintBoard();
            System.out.println("Completed stage: " + currentStep + ", Waiting for input...");
            try {
                System.in.read();
            } catch (IOException ex) {
                Logger.getLogger(Worker.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
    }

    public void PrintRedCells() {
        System.out.println("Red cells from worker: " + workerID);
        String line;

        line = "\t";
        for (int i = 0; i < myRedCells.length; i++) {
            line += myRedCells[i].contents();
        }
        System.out.println(line);
    }

    public void PrintBlueCells() {
        System.out.println("Blue cells from worker: " + workerID);
        String line;

        line = "\t";
        for (int i = 0; i < myBlueCells.length; i++) {
            line += myBlueCells[i].contents();
        }
        System.out.println(line);
    }

    public void PrintTiles() {
        System.out.println("Tiles from worker: " + workerID);
        System.out.println("\tThis isn't actually implemented ATM.");
    }
}
