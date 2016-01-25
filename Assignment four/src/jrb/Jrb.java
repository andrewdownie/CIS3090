package jrb;

import java.io.FileNotFoundException;
import java.io.PrintWriter;
import java.io.UnsupportedEncodingException;
import java.text.DecimalFormat;
import java.util.concurrent.CyclicBarrier;
import java.util.logging.Level;
import java.util.logging.Logger;


/**
 * Jrb.java
 * This class holds the board, creates the workers and prints the results of the
 * program run.
 * @author Andrew Downie - 0786342
 */
public class Jrb {
    //Barrier to sync the workers
    private static CyclicBarrier barrier;

    /**s
     * @param args the command line arguments
     */
    public static void main(String[] args) {  
        PrintWriter pw = null;
        Worker[] workers;
        Thread[] threads;
        CmdLineArgs cla;
        Board board;
        
        long startTime, endTime;
        DecimalFormat df;
        
        //PrintProgramHeader();
        
        cla = new CmdLineArgs(args);
        //cla.PrintAll();
        
        board = new Board(cla.boardSize(), cla.tileSize(), cla.seed());
       
        //board.PrintBoard();////////////////////////////////////////////////////////////////
        //System.exit(1);/////////////////////////////////////////////
      
        
        startTime = System.currentTimeMillis();
        
        //_Create Threads here
        barrier = new CyclicBarrier(cla.workerCount());
        workers = new Worker[cla.workerCount()];
        threads = new Thread[cla.workerCount()];
        for(int i = 0; i < cla.workerCount(); i++){
            workers[i] = new Worker(i, cla, board, barrier);
            threads[i] = new Thread(workers[i]);
            threads[i].start();
           
        }
        
        for(int i = 0; i < cla.workerCount(); i++){
            try {
                threads[i].join();
            } catch (InterruptedException ex) {
                Logger.getLogger(Jrb.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
        
        endTime = System.currentTimeMillis();
        
        //System.out.println("---- Final board ----");
        //board.PrintBoard();
        
       
        
        int stepsPassed = board.steps;
        if(stepsPassed == - 1){
            stepsPassed = cla.maxSteps();
        }
        
        int terminationPercent = (int)((board.colorCount()*100)/(cla.tileSize()*cla.tileSize()));
        
        df = new DecimalFormat("#0.00");
        //System.out.println("Time taken: " + df.format(StopwatchTime(startTime, endTime)));
        String lastLine = "p" + cla.workerCount() + " b" + cla.boardSize() + " t" + cla.tileSize() + " c" + cla.maxColorDensity() + " m" + cla.maxSteps() + " s" + cla.seed();
        lastLine += " Iterations: " + stepsPassed + " Termination Percent: " + terminationPercent + " Time: " + df.format(StopwatchTime(startTime, endTime)) + "\n";
        
        System.out.print(lastLine);
        
        
         try{
            pw = new PrintWriter("redblue.txt", "UTF-8");
            board.WriteBoard(pw);
            pw.append(lastLine);
        } catch (FileNotFoundException ex) {
            Logger.getLogger(Jrb.class.getName()).log(Level.SEVERE, null, ex);
        } catch (UnsupportedEncodingException ex) {
            Logger.getLogger(Jrb.class.getName()).log(Level.SEVERE, null, ex);
        }finally{
            pw.close();
        }
    }
    
    
    private static double StopwatchTime(long startTime, long endTime){
        long timeDiff = endTime - startTime;
        double seconds = (double)timeDiff / 1000;
        
        return seconds;//format this to be to two decimals
    }
    
    
}

