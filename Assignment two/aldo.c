/*
Created by: Andrew Downie, 
Created on: November 5th, 2015, 
Last modified: November 11th, 2015,
Course: CIS*3090 - A2, University of Guelph

Description: see aldo.h for file description.
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>

#include <Windows.h>
#include <string.h>
#include "aldo.h"
#include "wallclock.h"

//_The maximum number of images that can be in the '**images' array.
int MAX_IMAGES = 1024;
//_The maxmimum number of images that can be in the '**aldos' array.
int MAX_ALDOS = 512;

//_The array of images from the targets directory, and the number of images in the array.
Image **images;
int imageCount;

//_The array of images from the parallaldos directory, and the number of images in the array.
Image **aldos;
int aldoCount;

///_The paths to the folders containing the images, and the number of cores.
char *aldodir, *imagedir;
int cores;//Determines how many workers to create. If cores == 1, no workers created. Otherwise workers created = cores.

//_Barrier used to synchronize the workers when there is more one core specified in command line argument 3.
pthread_barrier_t barrier;


int main( int argc, char *argv[] )
{
	double programTime;
	//_Start timer ASAP.
	StartTime();
	

	//_Make sure there are enough command line arguments for the program to run properly.
	if(argc != 4){
		printf("ERROR>> Incorrect number of command line arguments\n");
		printf("\t%d arguments found, 3 required:\n", argc - 1);
		printf("\t\t1. aldodir\n");
		printf("\t\t2. imagedir\n");
		printf("\t\t3. cores\n");
		printf("\n");
		return EXIT_FAILURE;
	}

	//_Collect the program command line arguments.
	aldodir = argv[1];
	imagedir = argv[2];
	cores = atoi(argv[3]);

	//_Print the command line arguments for human verification.
	printf("Command line arguments:\n\t1. aldodir: %s\n\t2. imagedir: %s\n\t3. cores: %d\nStarting program...\n", aldodir, imagedir, cores);

	//_Verify that there were enough cores specified on the command line.
	if(cores < 1){
		printf("ERROR>> Must have at least one core to run program (argument 3 was: %d)\n\texiting...\n", cores);
		return EXIT_FAILURE;
	}

	//_To avoid having to do slow reallocs, just malloc a ton of space in each of the image arrays.
	images = (Image**)malloc(sizeof(Image*) * MAX_IMAGES);
	aldos = (Image**)malloc(sizeof(Image*) * MAX_ALDOS);

	//_Read the images into the image arrays.
	ReadImageFileNames(imagedir, "*.img", images, &imageCount);
	ReadImageFileNames(aldodir, "*.txt", aldos, &aldoCount);

	//_Print issues encountered while reading the images and then exit.
	if(imageCount > MAX_IMAGES){ printf("ERROR>> Trying to read in %d images, which is more than MAX_IMAGES. \n\tMAX_IMAGES=%d, please increase MAX_IMAGES.\n\n", imageCount, MAX_IMAGES); return EXIT_FAILURE;}
	if(aldoCount > MAX_ALDOS){ printf("ERROR>> Trying to read in %d aldos, which is more than MAX_ALDOS. \n\tMAX_ALDOS=%d, please increase MAX_ALDOS.\n\n", aldoCount, MAX_ALDOS); return EXIT_FAILURE;}
	if(imageCount == 0){ printf("Target directory appears to be incorrect empty, exiting...\n"); return EXIT_FAILURE;}
	if(aldoCount == 0){ printf("Aldo directory appears to be incorrect or empty, exiting...\n"); return EXIT_FAILURE;}

	//_Decide whether to run in parallel or serial mode.
	if(cores == 1){
		Serial(imagedir, images, imageCount, aldodir, aldos, aldoCount);
		PrintImageLocations(images, imageCount);
	}
	else{
		Parallel(cores, imagedir, images, imageCount, aldodir, aldos, aldoCount);
	}

	//_Stop timer, and then print out the timer recorded.
	programTime = EndTime();
	printf("Program execution time: %lf\n", programTime);//The spec did not specifiy how many decimal points to round to.

	//_Free everything
	FreeImageList(images, imageCount);
	FreeImageList(aldos, aldoCount);
	free(images);
	free(aldos);

	return 0;
}

//||---------------------------||
//||--Parallel Implementation--||
//||---------------------------||
void Parallel(int workerCount, char *imagedir, Image **images, int imageCount, char *aldodir, Image **aldos, int aldoCount){
	int i;
	
	pthread_attr_t attr;//_Attribute that will be applied to each thread.
	pthread_t *workers;//_List of threads which are to be workers.
	int *threadID;//_List ints because having to send an integers to a worker as a void * is dumb.

	//_Setup the attribute that will be applied to each thread.
	pthread_attr_init(&attr);
	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
	//_Setup the barrier we will use later
	pthread_barrier_init(&barrier, NULL, cores);

	//_Malloc the worker array and id array to fit the number of threads that will be created.
	workers = (pthread_t*)malloc(sizeof(pthread_t) * workerCount);
	threadID = (int*)malloc(sizeof(int) * workerCount);

	//_Create the threads, and send their id.
	for(i = 0; i < workerCount; i++){
		threadID[i] = i;
		pthread_create(&(workers[i]), &attr, ParallelWorker, (void *)&(threadID[i]));
	}
	
	//_Join the threads once they are done their work.
	for(i = 0; i < workerCount; i++){
		pthread_join(workers[i], NULL);
	}
}

void *ParallelWorker(void *workerID){
	int i, id;
	int rc;

	//_Convert the void * back to an int
	id = *((int*)workerID);


	//_Read the aldos
	for(i = aldoCount-1-id; i >= 0; i-=cores){
		ReadImage(aldodir, aldos[i]);
	}

	//_Read the images
	for(i = id; i < imageCount; i+=cores){
		ReadImage(imagedir, images[i]);
	}

	//_Wait for all other workers to be done file reading
	rc = pthread_barrier_wait(&barrier);
	if(rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD){
		printf("Could not wait on barrier!!\n");
	}

	//_Search through each image
	for(i = 0; i < imageCount; i++){
		ParallelImageSearch(id, images[i], aldos, aldoCount);
	}

	//_Wait for all other workers to be done image searching
	rc = pthread_barrier_wait(&barrier);
	if(rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD){
		printf("Could not wait on barrier!!\n");
	}

	//_Print results in parallel
	for(i = id; i < imageCount; i+=cores){
		printf("%s\n", images[i]->matchlocation);
	}

	
	return NULL;
}

void ParallelImageSearch(int workerID, Image *image, Image **aldos, int aldoCount){
	int xpos, ypos, ald;

	//_Go through each aldo, and look for them in this image
	for(ald = 0; ald < aldoCount; ald++){
		if(image->matchlocation[0] != '\0'){ return; }

		//_Search for every position that an aldo would fit in original rotation or flipped 180 degrees, on threadid-th row
		for(ypos = workerID; ypos < (image->height - aldos[ald]->height); ypos+=cores){
			if(image->matchlocation[0] != '\0'){ return; }
			for(xpos = 0; xpos < (image->width - aldos[ald]->width); xpos++){
				//if(image->matchlocation[0] != '\0'){ return; }
				if(Match000(image, aldos[ald], xpos, ypos)){
					return;
				}
				if(Match180(image, aldos[ald], xpos, ypos)){
					return;
				}
			}
		}

		if(image->matchlocation[0] != '\0'){ return; }
		//_Search for every position that an aldo would fit rotated 90 or 270 degrees from original position, on threadid-th row
		for(ypos = workerID; ypos < (image->height - aldos[ald]->width); ypos+=cores){
			if(image->matchlocation[0] != '\0'){ return; }
			for(xpos = 0; xpos < (image->width - aldos[ald]->height); xpos++){
				//if(image->matchlocation[0] != '\0'){ return; }
				if(Match090(image, aldos[ald], xpos, ypos)){
					return;
				}
				if(Match270(image, aldos[ald], xpos, ypos)){
					return;
				}
			}
		}
	}	
}

//||-------------------------||
//||--Serial Implementation--||
//||-------------------------||
void Serial(char *imagedir, Image **images, int imageCount, char *aldodir, Image **aldos, int aldoCount){
	int i, img;
	
	//_Read the images in
	for(i = 0; i < imageCount; i++){
		ReadImage(imagedir, images[i]);
	}

	//_Read the aldos in
	for(i = 0; i < aldoCount; i++){
		ReadImage(aldodir, aldos[i]);
	}

	//_Search through each image
	for(img = 0; img < imageCount; img++){
		SerialImageSearch(images[img], aldos, aldoCount);
	}
}

void SerialImageSearch(Image *image, Image **aldos, int aldoCount){
	int xpos, ypos, ald;

	//_Go through each aldo, and look for them in this image
	for(ald = 0; ald < aldoCount; ald++){

		//_Search for every position that an aldo would fit in original rotation or flipped 180 degrees
		for(ypos = 0; ypos < (image->height - aldos[ald]->height); ypos++){
			for(xpos = 0; xpos < (image->width - aldos[ald]->width); xpos++){
				if(Match000(image, aldos[ald], xpos, ypos)){
					return;
				}
				if(Match180(image, aldos[ald], xpos, ypos)){
					return;
				}
			}
		}

		//_Search for every position that an aldo would fit rotated 90 or 270 degrees from original position
		for(ypos = 0; ypos < (image->height - aldos[ald]->width); ypos++){
			for(xpos = 0; xpos < (image->width - aldos[ald]->height); xpos++){
				if(Match090(image, aldos[ald], xpos, ypos)){
					return;
				}
				if(Match270(image, aldos[ald], xpos, ypos)){
					return;
				}
			}
		}
	}
}



//||--------------||
//||--Match Aldo--||
//||--------------||
int Match000(Image *image, Image *aldo, int startx, int starty){
	int xpos, ypos;

	//_go through each pixel of the given aldo
	for(xpos = 0; xpos < aldo->width; xpos++){
		for(ypos = 0; ypos< aldo->height; ypos++){

			//_Compare our current pixel in the aldo, to the mapped pixel on the image
			if(aldo->pixels[ypos][xpos] != image->pixels[starty + ypos][startx + xpos]){
				return 0;
			}
		}
	}

	sprintf(image->matchlocation, "$%s %s (%d,%d,0)", aldo->filename, image->filename, starty + 1, startx + 1);
	//image->matchfound = 1;
	
	return 1;
}

int Match180(Image *image, Image *aldo, int startx, int starty){
	int xpos, ypos;
	int endx, endy;

	//_Figure out the end indexes of the aldo
	endx = startx + aldo->width - 1;
	endy = starty + aldo->height - 1;

	//_go through each pixel of the given aldo
	for(xpos = 0; xpos < aldo->width; xpos++){
		for(ypos = 0; ypos < aldo->height; ypos++){

			//_Compare our current pixel in the aldo, to the mapped pixel on the image
			if(aldo->pixels[ypos][xpos] != image->pixels[endy - ypos][endx - xpos]){
				return 0;
			}
		}
	}
	sprintf(image->matchlocation, "$%s %s (%d,%d,180)", aldo->filename, image->filename, endy + 1, endx + 1);
	//image->matchfound = 1;
	
	return 1;
}

int Match090(Image *image, Image *aldo, int startx, int starty){
	int xpos, ypos;
	int endx;

	//_Figure out the end index of the aldo
	endx = startx + aldo->height - 1;

	//_go through each pixel of the given aldo
	for(xpos = 0; xpos < aldo->width; xpos++){
		for(ypos = 0; ypos < aldo->height; ypos++){

			//_Compare our current pixel in the aldo, to the mapped pixel on the image
			if(aldo->pixels[ypos][xpos] != image->pixels[starty + xpos][endx - ypos]){
				return 0;
			}
		}
	}

	sprintf(image->matchlocation, "$%s %s (%d,%d,90)", aldo->filename, image->filename, starty + 1, endx + 1);
	//image->matchfound = 1;
	
	return 1;
}

int Match270(Image *image, Image *aldo, int startx, int starty){
	int xpos, ypos;
	int endy;

	//_Figure out the end index of the aldo
	endy = starty + aldo->width - 1;

	//_go through each pixel of the given aldo
	for(xpos = 0; xpos < aldo->width; xpos++){
		for(ypos = 0; ypos< aldo->height; ypos++){

			//_Compare our current pixel in the aldo, to the mapped pixel on the image
			if(aldo->pixels[ypos][xpos] != image->pixels[endy - xpos][startx + ypos]){
				return 0;
			}
		}
	}

	sprintf(image->matchlocation, "$%s %s (%d,%d,270)", aldo->filename, image->filename, endy + 1, startx + 1);
	//image->matchfound = 1;
	
	return 1;
}

