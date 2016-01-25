/*
Created by: Andrew Downie, 
Created on: November 6th, 2015, 
Last modified: November 11th, 2015,
Course: CIS*3090 - A2, University of Guelph

Description: see ImageReader.h for file description.
*/

#include <stdio.h>
#include <Windows.h>
#include <string.h>
#include "ImageReader.h"

//||------------------------||
//||--Forward Declarations--||
//||------------------------||
int _ReadImageDimensions(FILE *fp, Image *image);
FILE* _OpenImage(char *directoryPath, char *imageName);


//||--------------------||
//||--Public Functions--||
//||--------------------||

void FreeImageList(Image **images, int count){
	int i, row;

	for(i = 0; i < count; i++){

		//_Free each row.
		for(row = 0; row < images[i]->height; row++){
			free(images[i]->pixels[row]);
		}

		//_Free everything else.
		free(images[i]->pixels);
		free(images[i]);
	}
}

void PrintImageLocations(Image **images, int imageCount){
	int i;

	for(i = 0; i < imageCount; i++){
		//_If the match location isn't a null terminator, we must of found an aldo in the image, and so we can print that we found it.
		if(images[i]->matchlocation[0] != '\0'){
			printf("%s\n", images[i]->matchlocation);
		}
		/*else{
			printf("!!>> No aldo found for image: %s.\n", images[i]->filename);
		}*/
		
	}
}

void ReadImage(char *directory, Image *image){
	FILE *fp;
	int row, firstLineSize;

	//_Open the image file
	fp = _OpenImage(directory, image->filename);

	//_Read the images first line to get the images dimensions, save the length of the first line
	firstLineSize = _ReadImageDimensions(fp, image);

	//_allocate the number of rows we need
	image->pixels = (char**)malloc(sizeof(char*) * image->height);

	//_Go through each row, allocate the length of each row, read data from the image into the row, and terminate it.
	for(row = 0; row < image->height; row++){
		image->pixels[row] = (char*)malloc(sizeof(char) * (image->width + 1));
		fread(image->pixels[row], image->width+1, sizeof(char), fp);
		image->pixels[row][image->width] = '\0';
	}
	
	image->matchlocation[0] = '\0';

	fclose(fp);
}

void ReadImageFileNames(char *directory, char *searchPattern, Image **images, int *imageCount){
	WIN32_FIND_DATA FindFileData;
    HANDLE hFind;
	char fulldir[200];

	//_Create the full path with the search pattern we will use to open the files.
	sprintf(fulldir, "%s\\%s", directory, searchPattern);
	*imageCount = 0;

	//_Find our first image file
	 hFind = FindFirstFile(fulldir, &FindFileData);
	 if (hFind == INVALID_HANDLE_VALUE) {
		 //_Close thre handle if we didn't find anything.
		FindClose(hFind);
		return;
		
    } else {
		//_Read the data from the image file into an image struct
		images[*imageCount] = (Image*)malloc(sizeof(Image));
		images[*imageCount]->filename = (char*)malloc(sizeof(char) * strlen(FindFileData.cFileName));
		strcpy(images[*imageCount]->filename, FindFileData.cFileName);
		
		//images[*imageCount]->filesize = FindFileData.nFileSizeLow;

		//_increment the number of images we've found
		*imageCount = *imageCount + 1;
    }
	
	 //_Once we found the first image file, keep finding the next image file, until there are no more images in the directory.
	while(1){
		if(FindNextFile(hFind, &FindFileData)){
			//_Read the data from the image file into an image struct
			images[*imageCount] = (Image*)malloc(sizeof(Image));
			images[*imageCount]->filename = (char*)malloc(sizeof(char) * strlen(FindFileData.cFileName));
			strcpy(images[*imageCount]->filename, FindFileData.cFileName);

			//images[*imageCount]->filesize = FindFileData.nFileSizeLow;

			//_increment the number of images we've found
			*imageCount = *imageCount + 1;
		}
		else{
			 //_Close thre handle if we didn't find anything.
			FindClose(hFind);
			return;
			
		}
	}
	FindClose(hFind);
    return;
}

//||--------------------||
//||--Helper Functions--||
//||--------------------||
/*_ReadImageDimensions
	Reads the first line of the image file, and gets the width and height.
*/
int _ReadImageDimensions(FILE *fp, Image *image){
	//_Read height
	char imageDimension[10];
	char curChar = '\0';
	int index = 0;
	int firstLineSize = 0;

	while(curChar != ' '){
		curChar = fgetc(fp);
		firstLineSize ++;
		
		imageDimension[index] = curChar;
		index++;
		
	}
	imageDimension[index] = '\0';
	image->height = atoi(imageDimension);

	//_Read width
	index = 0;
	imageDimension[index] = '\0';
	while(curChar != '\n'){
		curChar = fgetc(fp);
		firstLineSize ++;
		
		imageDimension[index] = curChar;
		index++;
	}
	imageDimension[index] = '\0';
	image->width = atoi(imageDimension);

	return firstLineSize;
}
/*_OpenImage
	Concatenates the directory and image name, and then opens the resulting path.
*/
FILE* _OpenImage(char *directoryPath, char *imageName){
	char fullpath[200];
	int pathLen;
	
	pathLen = strlen(directoryPath) + strlen(imageName) + 1;
	sprintf(fullpath, "%s\\%s", directoryPath, imageName);

	return fopen(fullpath, "r");
}