//CIS*3090, Andrew Downie - 0786342
//Created: September 27th, 2015
//Last Modified: October 14th, 2015

/*FILE: pal.h
    Submission for CIS*3090 a1.
    A collection of functions for figuring out what lines in a file are pallindromes.

    Allows for two methods of processing the input file: serial using 1 processor,
    and parallel using 2 or more processors. Program will automatically detect whether
    to run in serial or parallel mode.

    Results of program are output to a file called "pal.out", this file contains
    a list of numbers, each number representing a line from the original file that
    was a pallindrome.

*/

void SetupWorkers(int workerCount);

void Serial(FILE *inputFile, FILE *outputFile);

void Parallel(FILE *inputFile, FILE *outputFile, int workers);

int LineIsPalindrome(char *line);
