//CIS*3090, Andrew Downie - 0786342
//Created: September 27th, 2015
//Last Modified: October 14th, 2015
//Check header for desciption.

#include <stdlib.h>
#include <stdio.h>
#include "pilot.h"
#include "pal.h"

#define MAX_OUTPUT_LINE_LENGTH 11	// 10 digits + \n
#define LINES_BEFORE_SENDING 100	// The number of lines to collect from the file, before sending off to a worker.
#define MAX_LINE_LENGTH 201		 	// 199 chars + \0 + \n. The longest a line from the input file, can be.
#define READY_FOR_DATA 'R'			// Meaningless value, workers send this to PI_Main to signal they are ready for more work.
#define END_OF_DATA -1				// Flag to tell worker they are done, sent in place of a starting line number

PI_BUNDLE *fromWorkerBundle;	// Bundle from workers to PI_Main
PI_CHANNEL **toWorker;			// array of channel pointers, from PI_Main to any worker
PI_CHANNEL **result;			// array of channel pointers, from any worker, to PI_Main
PI_PROCESS **Worker;			// array of process pointers

/*
	Performs all the work in the parallel version of this program. Is not run in serial version of this program (when workerCount == 0).
	- Loops accepting data from PI_Main until END_OF_DATA message is recieved.
	- Each time data is recieved the data is processed, the results appended to the variable 'pallindromes' and then this worker tells PI_Main it is ready for more data.
	- When END_OF_DATA message is recieved, this worker sends PI_Main the contents of the variable 'pallindromes'.
*/
int workerFunc( int index, void* arg2 )
{
	int pallindromesCount = 0;// How many pallindromes have been found so far
	int maxPallindromes = 32; // The maximum number of pallindromes that can be held by this worker. This will grow dynamically to keep up as the worker finds more and more pallindromes.
	int startingLine = 0;	  // The starting line of the data sent from PI_Main. Data from PI_Main is assumed to be a continous set of lines from the input file.
	int dataLength = 0;		  // The number of chars (+ 1 for '\0') contained in the last data sent from PI_Main.
	int lineIndex = 0;		  // The current index of a temporary string, used to parse the data from PI_Main. The string this indexes is sent to function 'LineIsPalindrome()'.
	int curLine;			  // The running counter of what line we are on in the file. 'curLine' starts at startingLine, and is incremented everytime a '\n' is found in variable 'data' from PI_Main.
	int i;					  // Generic iterator variable, used to loop through the data sent from PI_Main.

	char line[MAX_LINE_LENGTH];	// A single line from the input file. Parsed out of the data sent from PI_Main in the variable 'data'.
	char *pallindromes;			// The output from this worker, 'pallindromes' will be returned to PI_Main when this workers recieves END_OF_DATA as a line number.
								// Simply a list of all the lines in the input file that are pallindromes, seperated by newlines.
	char *data = NULL;			// The data sent from PI_Main, LINES_BEFORE_SENDING number of lines or less. This is a continous string, each line must be parsed out by looking for '\n's.


	// Malloc pallindromes to a default starting size
	pallindromes = (char *)malloc(sizeof(char) * maxPallindromes * MAX_OUTPUT_LINE_LENGTH);


	while(1){
		// Tell PI_Main this worker is ready for data.
		PI_Write( result[index], "%c", READY_FOR_DATA);
		// Wait until PI_Main sends this worker something.
		PI_Read( toWorker[index], "%d", &startingLine);
		// If PI_Main sends us the END_OF_DATA signal, we're done, otherwise we should work on the data recieved.
		if(startingLine == END_OF_DATA){
			break;
		}

		// If PI_Main didn't send us END_OF_DATA, then PI_MAIN must have data for us to work on.
		PI_Read( toWorker[index], "%^s", &dataLength, &data );


		// Once we have the data, initalize the workers line counter.
		curLine = startingLine;
		// Go through each char of the data recieved in order to parse it.
		for(i = 0; i < dataLength; i++){
			// Copy the data from PI_Main into a single line.
			line[lineIndex] = data[i];
			lineIndex++;

			// If we encounter a char that represents the end of a line, we have a line, and we should figure out if it is a pallindrome.
			if(data[i] == '\n' || data[i] == '\0'){
				line[lineIndex - 1] = '\0';// Cap the line, so it becomes a proper C style string.

				if(LineIsPalindrome(line)){

					if(pallindromesCount >= maxPallindromes){	// If we've run out of space to store line numbers of lines that are pallindromes,
						maxPallindromes = maxPallindromes * 2;	// have this worker allocate more space. Double the amount of space we allocate each time in an attempt to
						pallindromes = (char *)realloc(pallindromes, sizeof(char) * maxPallindromes * MAX_OUTPUT_LINE_LENGTH); // balance performance and memory usage.
						if(pallindromes == NULL){
							ExitFailure("Error reallocing...", __FILE__, __LINE__);
						}
					}

					// Save the line number to our workers output string, (if it is a pallindrome), and increment the number of pallindromes we've found.
					sprintf(&(pallindromes[pallindromesCount * MAX_OUTPUT_LINE_LENGTH]), "%010d\n", curLine);
					pallindromesCount++;

				}
				// Prepare to continue reading the data sent from PI_Main.
				curLine++;
				line[0] = '\0';
				lineIndex = 0;
			}
		}

		free(data);

	}
	printf(">> Worker %d recieived end of data message\n\n", index);

	// Send PI_Main the results of this workers work.
	PI_Write( result[index], "%^s", strlen(pallindromes) + 1, pallindromes);
	free(pallindromes);
	return 0;
}

/*
	Function to allow consistant way of exiting the program when an error is encountered.
*/
int ExitFailure(char *exitMessage, char *file, int line){
	printf("\nERROR: %s\n\n", exitMessage);
	PI_Abort(EXIT_FAILURE, ", Error: No input file specified.", file, line);
	return EXIT_FAILURE;
}

int main( int argc, char *argv[] )
{
	FILE *output, *input;	// File pointers for the data that will be read in and written out.
	double programRunTime; 	// The amount of time a this program took to complete execution.
	int processCount;		// The total number of processes we have including PI_Main.
	int workerCount;		// The max number of workers this program run can utilize.

	// Setup our program, calculate how many workers we have.
	processCount = PI_Configure( &argc, &argv );
	workerCount = processCount - 1;

	PI_StartTime();

	if(argc != 2){ // If after setting up our program, we don't have a string for a file argument, abort this run.
		return ExitFailure(", Error: No input file specified.", __FILE__, __LINE__);
	}

	// Create the workers based off worker count, and the channels/bundles between the workers and PI_Main.
	SetupWorkers(workerCount);

	//**********//
	PI_StartAll();
	//**********//

	// File opening
	output = fopen("pal.out", "a");
	input = fopen(argv[1], "r");

	// File error checking.
	if(output==NULL){
		return ExitFailure("Could not open output file 'pal.out' for appending in current directory.", __FILE__, __LINE__);
	}
	if(input==NULL){
		return ExitFailure("Could not open input file for reading, from path in argv[1].", __FILE__, __LINE__);
	}
	else{
		printf(">> Input file is: '%s'\n", argv[1]);
	}


	// Run serial or parallel algorithm.
	if(workerCount == 0){
		Serial(input, output);
	}
	else{
		Parallel(input, output, workerCount);
	}

	// Clean up our program run.
	fclose(output);
	fclose(input);

	free(toWorker);
	free(Worker);
	free(result);

	printf(">> Exiting... <<\n");

	programRunTime = PI_EndTime();
	printf("\n\n>>< PROGRAM EXECUTION TIME: (%.2f)s ><<\n\n\n", programRunTime);
	PI_StopMain(0);
	return EXIT_SUCCESS;
}

/*
	Creates the workers. Setups up channels from the workers to PI_MAIN.
	Setups up channels to the workers from PI_Main. Bundles the channels
	from the workers to PI_Main.
*/
void SetupWorkers(int workerCount){
	int i;

	// If there are no workers, we have nothing to setup, so just leave.
	if(workerCount == 0){return;}

	Worker = malloc( workerCount * sizeof(PI_PROCESS*) );
	toWorker = malloc( workerCount * sizeof(PI_CHANNEL*) );
	result = malloc( workerCount * sizeof(PI_CHANNEL*) );

	for ( i=0; i<workerCount; i++ ) {
		Worker[i] = PI_CreateProcess( workerFunc, i, NULL );
		toWorker[i] = PI_CreateChannel( PI_MAIN, Worker[i] );
		result[i] = PI_CreateChannel( Worker[i], PI_MAIN );
	}

	fromWorkerBundle = PI_CreateBundle(PI_SELECT, result, workerCount);
}

/*
	The serial version of this program.
	Loops while theres still more to read in the file. Each time a line is found,
	the found line is checked if it is a pallindrome. If it is, the line number of
	the line that is a pallindrome is written to the output on the spot.
*/
void Serial(FILE *inputFile, FILE *outputFile){
	char buffer[MAX_LINE_LENGTH];	// A buffer used to store a single line from the input file.
	char curChar = ' ';				// The current character read in from the input file.

	int bufferIndex = 0;// The number of characters currently in the 'buffer' variable.
	int curLine = 1;	// The line we are on from the input file.

	printf(">> STARTING SERIAL ALGO...\n");

	while (curChar != EOF) {
		// Go through the input file, gathering one line at a time.
		curChar = fgetc(inputFile);
		buffer[bufferIndex] = curChar;
		bufferIndex++;

		// When a line is found, check if it is a pallindrome,
		if(curChar == '\n' || curChar == '\0'){
			buffer[bufferIndex - 1] = '\0';

			// if the line is a pallindrome, wright it to the output file right away.
			if(LineIsPalindrome(buffer)){
				fprintf(outputFile, "%010d\n", curLine);
			}

			// Prepare for the next line to be found.
			buffer[0] = '\0';
			bufferIndex = 0;
			curLine++;
		}
	}

	printf(">> ENDING SERIAL ALGO... <<\n");
}

/*
	Part of the parallel verison of this program. This function acts as 'PI_Main' in the master/worker configuration.
	- Loops while there is still data in the input file, and sends a chunck of data off to a worker,
	as soon a chunk is read in from the input file. (Chunk size is defined by LINES_BEFORE_SENDING).
	- Once the end of the input file is reached, sends a signal to every worker that there is no more data left.
	- Once all workers have recieved the END_OF_DATA signal, this function collects the data from each worker one by one,
	  writes said data to the output file right away.
*/
void Parallel(FILE *inputFile, FILE *outputFile, int workerCount){
	printf(">> STARTING PARALLEL...\n");

	char buffer[LINES_BEFORE_SENDING * MAX_LINE_LENGTH]; // Buffer which is the maximum size we can send to a worker in a single chunk.
	char *workerResult = NULL;	// The data gotten back from a worker, after the worker is told they are done. The output of this program.
	char curChar = ' ';			// The current char read in from the input file.
	char signal = ' ';			// Char recieved from worker, to tell PI_Main (this function) that the sending worker is ready for more data.

	int workerResultLength = 0;	// The length of the result from the worker.
	int linesSinceSending = 0;	// The number of lines we've read in from the input file. Resets to 0 everytime we send a chunk of data/lines to a worker.
	int nextChannel = 0;		// The next channel that is able to accept data from PI_Main (this function).
	int bufferPos = 0;			// The number of chars currently in the line buffer, once we find a newline this is reset to zero.
	int lineNum = 1;			// The current line number we are on in the input file.
	int i;	//Generic iterator, used to go through each channel to tell workers they are done, and then collect thier data.

	while(curChar != EOF){
		curChar = fgetc(inputFile);

		// Read the file in char by char, and construct the file into lines, one line at a time.
		if(curChar != '\0'){// Ignore null terminators in the buffer.
			buffer[bufferPos] = curChar;
			bufferPos++;
		}

		// When we find a line, increment the number of lines we've found since last time we sent a chunk of lines.
		if(curChar == '\n' || curChar == '\0' || curChar == EOF){
			linesSinceSending++;

			// If the threshold number of lines is reached, send the chunk of lines to the next available worker.
			if(linesSinceSending >= LINES_BEFORE_SENDING || curChar == EOF){
				buffer[bufferPos-1] = '\0';

				// Find the next available worker, and read their ready message.
				nextChannel = PI_Select(fromWorkerBundle);
				PI_Read(result[nextChannel], "%c", &signal);

				// Send them the line number we started at since last time a chunk of lines was sent, and then send the worker the actual chunk of lines.
				PI_Write( toWorker[nextChannel], "%d", lineNum);
				PI_Write( toWorker[nextChannel], "%^s", strlen(buffer) + 1, &buffer);


				// Get ready to continue collecting lines and sending chunks of lines to workers.
				buffer[0] = '\0';
				bufferPos = 0;
				lineNum += linesSinceSending;
				linesSinceSending = 0;
			}
		}

	}
	printf(">> Telling workers they are done.\n");
	// Send workers signal that there is no more data to work on.
	for(i = 0; i < workerCount; i++){
		PI_Read(result[i], "%c", &signal);
		PI_Write( toWorker[i], "%d", END_OF_DATA);
	}
	printf(">> Collecting data from workers.\n");
	// Gather the results from each worker, and write the results to a file as they are gotten.
	for(i = 0; i < workerCount; i++){
		PI_Read(result[i], "%^s", &workerResultLength, &workerResult);
		fprintf(outputFile, "%s", workerResult);
		free(workerResult);
	}
	printf(">> ENDING PARALLEL... <<\n");
}

/*
	Takes a C styled string:
	- returns 1 if the given string is a pallindrome,
	- returns 0 in all other cases.
*/
int LineIsPalindrome(char *line){
	char backwardsLine[MAX_LINE_LENGTH]; // A copy of the input line, but backwards
	int lineLen; // The length of the input line given.
	int i;		 // Generic iterator, used to go through the input line, and copy the reverse of it to 'backwardsLine'.

	// Create a copy of the line, but backwards.
	lineLen = strlen(line);
	for(i = 0; i < lineLen; i++){
		backwardsLine[i] = line[lineLen - i - 1];
	}
	backwardsLine[lineLen] = '\0';

	// Check if the backwards line, is the same as the normal line.
	// If the backwards line is the same as the normal line, a pallindrome was found.
	if(strcmp(backwardsLine, line) == 0 && lineLen != 0){
		return 1;
	}
	return 0;
}
