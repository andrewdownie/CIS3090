/*
Created by: Andrew Downie, 
Created on: November 6th, 2015, 
Last modified: November 11th, 2015,
Course: CIS*3090 - A2, University of Guelph

Description: Holds defintion for an 'Image' struct (below). 
			 Allows file names of images in a directory to be read in.
			 Allows a single image to be read in with the full path to the image.
*/

typedef struct{
	int width, height;
	char **pixels;

	char *filename;
	//int filesize;

	//int matchfound;
	char matchlocation[100];
} Image;

/*FUNCTION: ReadImageFileNames
	 Given a path to a directory, and a pattern (example: "*.txt"), the file name of each file matching that pattern will be collected.
	 Collection of the file names will occur by populating each 'filename' attribute in the 'images' parameter. 'imageCount' will be set
	 to the number of file names saved into the images array.
*/
void ReadImageFileNames(char *directory, char *searchPattern, Image **images, int *imageCount);

/*FUNCTION: ReadImage
	Given a directory, and an image that has it's 'filename' attribute to a valid value, this function will
	populate the Image struct with info found in the file, excluding the attribute 'matchlocation'.
*/
void ReadImage(char *directory, Image *image);

/*FUNCTION: PrintImageLocations
	Given a list of Images and the number of images to print from that list, 
	this function will print each images 'matchlocation' attribute to standard out, each on its own line.
*/
void PrintImageLocations(Image **images, int imageCount);


/*FUNCTION: FreeImageList
	Frees all the pointers contained by the given images.
*/
void FreeImageList(Image **images, int count);