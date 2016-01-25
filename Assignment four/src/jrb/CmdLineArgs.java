package jrb;

/**
 *  CmdLineArgs.java
 *  This class parses the users input, and stores the values passed in for easy
 *  future use.
 * @author Andrew Downie - 0786342
 */
public class CmdLineArgs {
    //_All the info sent in from the command line
    private int workerCount = -1;
    private int boardSize = -1;
    private int tileSize = -1;
    private int maxColorDensity = -1;
    private int maxSteps = -1;
    private long seed = -1;
    private boolean seedProvided;
    
    
    //_Takes the programs command line arguments, verifies them,
    //_and packages them for future use
    public CmdLineArgs(String[] commandLineArgs){
        ParseArgs(commandLineArgs);
        VerifyArgs();
        
        if(!seedProvided){
           seed = System.currentTimeMillis();
        }
        
    }
    
    //_Handles the converting of the command line arguments from a string,
    //_to the correct values.
    private void ParseArgs(String[] commandLineArgs){
        String curStart;
        
        seedProvided = false;

        for(int i = 0; i < commandLineArgs.length; i++){
           curStart = commandLineArgs[i].substring(0,1);
                   
            switch(curStart.toLowerCase()){
                case "p":
                    workerCount = ArgToInt(commandLineArgs[i]);
                    break;
                case "b":
                    boardSize = ArgToInt(commandLineArgs[i]);;
                    break;
                case "t":
                    tileSize = ArgToInt(commandLineArgs[i]);
                    break;
                case "c":
                    maxColorDensity = ArgToInt(commandLineArgs[i]);;
                    break;
                case "m":
                    maxSteps = ArgToInt(commandLineArgs[i]);;
                    break;
                case "s":
                    seed = ArgToLong(commandLineArgs[i]);
                    seedProvided = true;
                    break;
            }
        }
        
        
    }
    
    //_Verifies the args after they have been read in, and converted to their
    //_Correct types
    private void VerifyArgs(){
        boolean invalidArgs = false;
        String output = "";
        
        if(workerCount < 1){
            output += "\tArgument:  processorCount(p) must be greater than 0,\n";
            invalidArgs = true;
        }
        
        if(boardSize < 2){
            output += "\tArgument:       boardSize(b) must be greater than 1,\n";
            invalidArgs = true;
        }
        
        if(tileSize > 0){
            if((boardSize*boardSize) % tileSize != 0){
                output += "\tArgument: boardSize(b) must be integer divisible by tileSize(t),\n";
                invalidArgs = true;
            }
        }
        else{
            output += "\tArgument:     overlaySize(t) must be greater than 0,\n";
            invalidArgs = true;
        }
        
        
        if(maxColorDensity < 1 || maxColorDensity > 100){
            output += "\tArgument: maxColorDensity(c) must be an integer between 1 and 100,\n";
            invalidArgs = true;
        }
        
        if(maxSteps < 1){
            output += "\tArgument:        maxSteps(m) must be greater than 0,\n";
            invalidArgs = true;
        }
        
        
        
        if(invalidArgs){
            System.out.println(">> COMMAND LINE ARGUMENT ERRORS:");
            System.out.println(output);
            System.out.println("exiting...");
            System.exit(1);
        }
    }
    
    //_ArgToInt_//
    //_Takes: a string starting with a single letter, followed by a positive integer.
    //_Returns: the integer following the single letter.
    private int ArgToInt(String arg){
        if(arg.length() <= 1){ return -1; }
        
        String sub = arg.substring(1, arg.length());
        
        try {
             return Integer.parseInt(sub);
        } catch (NumberFormatException e) {
             System.out.println("Parsing parameter: " + arg + " to int failed, exiting...");
            System.exit(1);
        }
        return -1;
    }
    
    //_ArgToFloat_//
    //_Takes: a string starting with a single letter, followed by a positive real number.
    //_Returns: the real number following the single letter.
    private long ArgToLong(String arg){
     if(arg.length() <= 1){ return -1; }
        
        String sub = arg.substring(1, arg.length());
        
        try {
             return Long.parseLong(sub);
        } catch (NumberFormatException e) {
            System.out.println("Parsing parameter: " + arg + " to long failed, exiting...");
            System.exit(1);
        }
        return -1;
    }
    
    //_Getters
    public int workerCount(){
        return workerCount;
    }
    public int boardSize(){
        return boardSize;
    }
    public int tileSize(){
        return tileSize;
    }
    public int maxColorDensity(){
        return maxColorDensity;
    }
    public int maxSteps(){
        return maxSteps;
    }
    public long seed(){
        return seed;
    }
    
    //_Debugging stuff
    public void PrintAll(){
        System.out.println("Command line arguments:");
        System.out.println("\t processorCount(p):  " + workerCount);
        System.out.println("\t      boardSize(b):  " + boardSize);
        System.out.println("\t       tileSize(t):  " + tileSize);
        System.out.println("\tmaxColorDensity(c):  " + maxColorDensity);
        System.out.println("\t       maxSteps(m):  " + maxSteps);
        
        if(seedProvided){
            System.out.println("\t           seed(s):  " + seed);
        }
        else{
            System.out.println("\t           seed(s):  No seed provided.");
        }
       
        System.out.println("");
    }
}
