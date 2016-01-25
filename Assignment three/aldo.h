/*
Created by: Andrew Downie, 
Created on: November 5th, 2015, 
Last modified: November 11th, 2015,
Course: CIS*3090 - A2, University of Guelph

Description: core functionality for serching through two sets of images.
			 The first set of images are targets. The second set of images are aldos. 
			 This program looks through each target image, and checks to see if any of the aldo images are present, at any of 4 rotations (0, 90, 180 and 270 degrees).
*/

#include "ImageReader.h"

/*ParallelImageSearch
	Each worker goes through a cyclic selection of rows in the image looking for the aldo, in the given image.
	At possible place an aldo could fit on a row, each of the four 'Match###' functions to check if the aldo is
	present. If the aldo is found, the image is marked as solved by setting the images 'mathclocation' to where
	the aldo was found. Setting matchlocation allows the other threads to know the image is solved.
*/
void ImageSearch(int workerID, Image *image, Image **aldos, int aldoCount);

/*Match000
	Checks to see if the given aldo image is present at position (startx, starty), unrotated.
*/
int Match000(Image *image, Image *aldo, int startx, int starty);


/*Match180
	Checks to see if the given aldo image is present at position (startx, starty), rotated 180 degrees.
*/
int Match180(Image *image, Image *aldo, int startx, int starty);


/*Match090
	Checks to see if the given aldo image is present at position (startx, starty), rotated 90 degrees.
*/
int Match090(Image *image, Image *aldo, int startx, int starty);


/*Match270
	Checks to see if the given aldo image is present at position (startx, starty), rotated 270 degrees.
*/
int Match270(Image *image, Image *aldo, int startx, int starty);



