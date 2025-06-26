// imageBWTest - A program that performs some image processing.
//
// This program is an example use of the imageBW module,
// a programming project for the course AED, DETI / UA.PT
//
// You may freely use and modify this code, NO WARRANTY, blah blah,
// as long as you give proper credit to the original and subsequent authors.
//
// The AED Team <jmadeira@ua.pt, jmr@ua.pt, ...>
// 2024

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "imageBW.h"
#include "instrumentation.h"

int main(int argc, char* argv[]) {
  if (argc != 1) {
    fprintf(stderr, "Usage: %s  # no arguments required (for now)\n", argv[0]);
    exit(1);
  }

  // To initalize operation counters
  ImageInit();



Image image_1 = ImageCreate(128,128,WHITE);
Image image_2 = ImageCreate(256,256,WHITE);
Image image_3 = ImageCreate(512,512,WHITE);
Image image_4 = ImageCreate(1024,1024,WHITE);

Image image_5 = ImageCreate(128,128,BLACK);
Image image_6 = ImageCreate(256,256,BLACK);
Image image_7 = ImageCreate(512,512,BLACK);
Image image_8 = ImageCreate(1024,1024,BLACK);


printf("começa aqui");
/*
Image image_9 = ImageCreateChessboard(128,128,1,0);
Image image_10 = ImageCreateChessboard(256,256,1,0);
Image image_11 = ImageCreateChessboard(512,512,1,0);
Image image_12 = ImageCreateChessboard(1024,1024,1,0);


Image image_17 = ImageCreateChessboard(128,128,2,0);
Image image_18 = ImageCreateChessboard(256,256,2,0);
Image image_19 = ImageCreateChessboard(512,512,2,0);
Image image_20 = ImageCreateChessboard(1024,1024,2,0);
printf("para c = 4-----------------------------------------\n");
Image image_21 = ImageCreateChessboard(128,128,4,0);
Image image_22 = ImageCreateChessboard(256,256,4,0);
Image image_23 = ImageCreateChessboard(512,512,4,0);
Image image_24 = ImageCreateChessboard(1024,1024,4,0);

*/

Image image_10 = ImageCreateChessboard(8,8,1,0);
ImageRAWPrint(image_10);
/*
Image image_13 = ImageAND(image_1,image_5); 
Image image_14 = ImageAND(image_2,image_6);
Image image_15 = ImageAND(image_3,image_7);
Image image_16 = ImageAND(image_4,image_8);


Image image_25 = ImageAND(image_9,image_5); 
Image image_26 = ImageAND(image_10,image_6);
Image image_27 = ImageAND(image_11,image_7);
Image image_28 = ImageAND(image_12,image_8);

*/




  

  /* Creating and displaying some images
  Image white_image = ImageCreate(8, 8, WHITE);
  ImageRAWPrint(white_image);

  Image black_image = ImageCreate(8, 8, BLACK);
  ImageRAWPrint(black_image);

  Image image_1 = ImageNEG(white_image);
  ImageRAWPrint(image_1);

  ImageRLEPrint(image_1);

  
  Image image_2 = ImageReplicateAtBottom(white_image, black_image);
  ImageRAWPrint(image_2);


  printf("image_1 AND image_1\n");
  Image image_3 = ImageAND(image_1, image_1);
  ImageRAWPrint(image_3);

  printf("image_1 AND image_2\n");
  Image image_4 = ImageAND(image_1, image_2);
  ImageRAWPrint(image_4);

  printf("image_1 OR image_2\n");
  Image image_5 = ImageOR(image_1, image_2);
  ImageRAWPrint(image_5);

  printf("image_1 XOR image_1\n");
  Image image_6 = ImageXOR(image_1, image_1);
  ImageRAWPrint(image_6);

  printf("image_1 XOR image_2\n");
  Image image_7 = ImageXOR(image_1, image_2);
  ImageRAWPrint(image_7);

  Image image_8 = ImageReplicateAtRight(image_6, image_7);
  ImageRAWPrint(image_8);

  Image image_9 = ImageReplicateAtRight(image_6, image_6);
  ImageRAWPrint(image_9);

  Image image_10 = ImageHorizontalMirror(image_1);
  ImageRAWPrint(image_10);

  Image image_11 = ImageVerticalMirror(image_8);
  ImageRAWPrint(image_11);
  ImageRLEPrint(image_11);

  Image image_12 = ImageCreateChessboard(8,8,1,0);
  ImageRAWPrint(image_12);
  // Saving in PBM format
  // Housekeeping
  //ImageDestroy(&white_image);
  //ImageDestroy(&black_image);
  //ImageDestroy(&image_1);
  */
/*** 
  ImageSave(image_1, "image_1.pbm");
  ImageSave(image_2, "image_2.pbm");
  ImageSave(image_3, "image_3.pbm");
  ImageSave(image_4, "image_4.pbm");
  ImageSave(image_5, "image_5.pbm");
  ImageSave(image_6, "image_6.pbm");
  ImageSave(image_7, "image_7.pbm");
  ImageSave(image_8, "image_8.pbm");
  ImageSave(image_9, "image_9.pbm");
  ImageSave(image_10, "image_10.pbm");
  ImageSave(image_11, "image_11.pbm");
  ImageSave(image_12, "image_12.pbm");

  
  ImageDestroy(&image_2);
  ImageDestroy(&image_3);
  ImageDestroy(&image_4);
  ImageDestroy(&image_5);
  ImageDestroy(&image_6);
  ImageDestroy(&image_7);
  ImageDestroy(&image_8);
  ImageDestroy(&image_9);
  ImageDestroy(&image_10);
  ImageDestroy(&image_11);

***/


  return 0;
}
