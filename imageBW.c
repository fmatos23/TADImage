/// imageBW - A simple image processing module for BW images
///           represented using run-length encoding (RLE)
///
/// This module is part of a programming project
/// for the course AED, DETI / UA.PT
///
/// You may freely use and modify this code, at your own risk,
/// as long as you give proper credit to the original and subsequent authors.
///
/// The AED Team <jmadeira@ua.pt, jmr@ua.pt, ...>
/// 2024

// Student authors (fill in below):
// NMec:113726
// Name:Francisco Matos
// NMec:114560
// Name:Luís Rodrigues
//
// Date:28/11/2024
//

#include "imageBW.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "instrumentation.h"

// The data structure
//
// A BW image is stored in a structure containing 3 fields:
// Two integers store the image width and height.
// The other field is a pointer to an array that stores the pointers
// to the RLE compressed image rows.
//
// Clients should use images only through variables of type Image,
// which are pointers to the image structure, and should not access the
// structure fields directly.

// Constant value --- Use them throughout your code
// const uint8 BLACK = 1;  // Black pixel value, defined on .h
// const uint8 WHITE = 0;  // White pixel value, defined on .h
const int EOR = -1;  // Stored as the last element of a RLE row

// Internal structure for storing RLE BW images
struct image {
  uint32 width;
  uint32 height;
  int** row;  // pointer to an array of pointers referencing the compressed rows
};

// This module follows "design-by-contract" principles.
// Read `Design-by-Contract.md` for more details.

/// Error handling functions

// In this module, only functions dealing with memory allocation or
// file (I/O) operations use defensive techniques.
// When one of these functions fails,
// it immediately prints an error and exits the program.
// This fail-fast approach to error handling is simpler for the programmer.

// Use the following function to check a condition
// and exit if it fails.

// Check a condition and if false, print failmsg and exit.
static void check(int condition, const char* failmsg) {
  if (!condition) {
    perror(failmsg);
    exit(errno || 255);
  }
}

/// Init Image library.  (Call once!)
/// Currently, simply calibrate instrumentation and set names of counters.
void ImageInit(void) {  ///
  InstrCalibrate();
  InstrName[0] = "pixmem";  // InstrCount[0] will count pixel array acesses
  // Name other counters here...
}

// Macros to simplify accessing instrumentation counters:
#define PIXMEM InstrCount[0]
// Add more macros here...

// TIP: Search for PIXMEM or InstrCount to see where it is incremented!

/// Auxiliary (static) functions

/// Create the header of an image data structure
/// And allocate the array of pointers to RLE rows
static Image AllocateImageHeader(uint32 width, uint32 height) {
  assert(width > 0 && height > 0);
  Image newHeader = malloc(sizeof(struct image));
  check(newHeader != NULL, "malloc");

  newHeader->width = width;
  newHeader->height = height;

  // Allocating the array of pointers to RLE rows
  newHeader->row = malloc(height * sizeof(int*));
  check(newHeader->row != NULL, "malloc");

  return newHeader;
}

/// Allocate an array to store a RLE row with n elements
static int* AllocateRLERowArray(uint32 n) {
  assert(n > 2);
  int* newArray = malloc(n * sizeof(int));
  check(newArray != NULL, "malloc");

  return newArray;
}

/// Compute the number of runs of a non-compressed (RAW) image row
static uint32 GetNumRunsInRAWRow(uint32 image_width, const uint8* RAW_row) {
  assert(image_width > 0);
  assert(RAW_row != NULL);

  // How many runs?
  uint32 num_runs = 1;
  for (uint32 i = 1; i < image_width; i++) {
    if (RAW_row[i] != RAW_row[i - 1]) {
      num_runs++;
    }
  }

  return num_runs;
}

/// Get the number of runs of a compressed RLE image row
static uint32 GetNumRunsInRLERow(const int* RLE_row) {
  assert(RLE_row != NULL);

  // Go through the RLE_row until EOR is found
  // Discard RLE_row[0], since it is a pixel color

  uint32 num_runs = 0;
  uint32 i = 1;
  while (RLE_row[i] != EOR) {
    num_runs++;
    i++;
  }

  return num_runs;
}

/// Get the number of elements of an array storing a compressed RLE image row
static uint32 GetSizeRLERowArray(const int* RLE_row) {
  assert(RLE_row != NULL);

  // Go through the array until EOR is found
  uint32 i = 0;
  while (RLE_row[i] != EOR) {
    i++;
  }

  return (i + 1);
}

/// Compress into RLE format a RAW image row
/// Allocates and returns the array storing the image row in RLE format
static int* CompressRow(uint32 image_width, const uint8* RAW_row) {
  assert(image_width > 0);
  assert(RAW_row != NULL);

  // How many runs?
  uint32 num_runs = GetNumRunsInRAWRow(image_width, RAW_row);

  // Allocate the RLE row array
  int* RLE_row = malloc((num_runs + 2) * sizeof(int));
  check(RLE_row != NULL, "malloc");

  // Go through the RAW_row
  RLE_row[0] = (int)RAW_row[0];  // Initial pixel value
  uint32 index = 1;
  int num_pixels = 1;
  for (uint32 i = 1; i < image_width; i++) {
    if (RAW_row[i] != RAW_row[i - 1]) {
      RLE_row[index++] = num_pixels;
      num_pixels = 0;
    }
    num_pixels++;
  }
  RLE_row[index++] = num_pixels;
  RLE_row[index] = EOR;  // Reached the end of the row

  return RLE_row;
}

static uint8* UncompressRow(uint32 image_width, const int* RLE_row) {
  assert(image_width > 0);
  assert(RLE_row != NULL);

  // The uncompressed row
  uint8* row = (uint8*)malloc(image_width * sizeof(uint8));
  check(row != NULL, "malloc");

  // Go through the RLE_row until EOR is found
  int pixel_value = RLE_row[0];
  uint32 i = 1;
  uint32 dest_i = 0;
  while (RLE_row[i] != EOR) {
    // For each run
    for (int aux = 0; aux < RLE_row[i]; aux++) {
      row[dest_i++] = (uint8)pixel_value;
    }
    // Next run
    i++;
    pixel_value ^= 1;
  }

  return row;
}

// Add your auxiliary functions here...

/// Image management functions

/// Create a new BW image, either BLACK or WHITE.
///   width, height : the dimensions of the new image.
///   val: the pixel color (BLACK or WHITE).
/// Requires: width and height must be non-negative, val is either BLACK or
/// WHITE.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageCreate(uint32 width, uint32 height, uint8 val) {
  assert(width > 0 && height > 0);
  assert(val == WHITE || val == BLACK);

  Image newImage = AllocateImageHeader(width, height);

  // All image pixels have the same value
  int pixel_value = (int)val;

  // Creating the image rows, each row has just 1 run of pixels
  // Each row is represented by an array of 3 elements [value,length,EOR]
  for (uint32 i = 0; i < height; i++) {
    newImage->row[i] = AllocateRLERowArray(3);
    newImage->row[i][0] = pixel_value;
    newImage->row[i][1] = (int)width;
    newImage->row[i][2] = EOR;
  }

  return newImage;
}

/// Create a new BW image, with a perfect CHESSBOARD pattern.
///   width, height : the dimensions of the new image.
///   square_edge : the lenght of the edges of the sqares making up the
///   chessboard pattern.
///   first_value: the pixel color (BLACK or WHITE) of the
///   first image pixel.
/// Requires: width and height must be non-negative, val is either BLACK or
/// WHITE.
/// Requires: for the squares, width and height must be multiples of the
/// edge lenght of the squares
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageCreateChessboard(uint32 width, uint32 height, uint32 square_edge, uint8 first_value) {
    // Instrumentação: medir tempo de execução
    double start_time = cpu_time();
    InstrCount[0] = 0;  // Contador para acessos à memória
    InstrCount[1] = 0;  // Contador para operações lógicas

    uint32 total_runs = 0; // Inicializa o contador de runs

    // Reserva a estrutura principal para a imagem
    Image img = AllocateImageHeader(width, height);
    if (img == NULL) {
        return NULL; // Retorna NULL se a alocação falhar
    }

    // Define a cor inicial da primeira linha
    uint8 starting_color = first_value;

    // Loop para processar cada linha
    for (uint32 i = 0; i < height; i++) {
        // Cria uma linha temporária
        uint8* temp_row = calloc(width, sizeof(uint8));
        InstrCount[0] += width; // Contar reserva de memória para a linha
        if (temp_row == NULL) {
            // Liberta a imagem se a reserva falhar
            ImageDestroy(&img);
            return NULL;
        }

        uint8 current_color = starting_color;
        uint32 line_runs = 1; // Cada linha começa com pelo menos um run

        // Preenche a linha com o padrão de xadrez
        for (uint32 j = 0; j < width; j++) {
            temp_row[j] = current_color;
            InstrCount[0]++; // Contar acesso ao array (escrita)
            
            // Alterna a cor após atingir o tamanho do quadrado
            if ((j + 1) % square_edge == 0) {
                current_color = !current_color;
                InstrCount[1]++; // Contar operação lógica (XOR)
                
                // Incrementa o contador de runs quando a cor muda
                if (j + 1 < width) { // Evita contar mudança após o fim da linha
                    line_runs++;
                }
            }
        }

        // Atualiza o total de runs para a imagem
        total_runs += line_runs;

        // Compacta a linha e adiciona ao cabeçalho da imagem
        img->row[i] = CompressRow(width, temp_row);
        free(temp_row); // Liberta a memória temporária da linha
        InstrCount[0]++; // Contar libertação de memória

        // Alterna a cor inicial para a próxima linha, se necessário
        if ((i + 1) % square_edge == 0) {
            starting_color = !starting_color;
            InstrCount[1]++; // Contar operação lógica (XOR)
        }
    }

    // Instrumentação: tempo de execução
    double end_time = cpu_time();
    printf("Tempo de execução da função ImageCreateChessboard: %.6f segundos\n", end_time - start_time);
    InstrPrint();

    // Exibe o número total de runs
    printf("Número total de runs: %u\n", total_runs);

    return img; // Retorna a imagem gerada
}


/// Destroy the image pointed to by (*imgp).
///   imgp : address of an Image variable.
/// If (*imgp)==NULL, no operation is performed.
/// Ensures: (*imgp)==NULL.
/// Should never fail.
void ImageDestroy(Image* imgp) {
  assert(imgp != NULL);

  Image img = *imgp;

  for (uint32 i = 0; i < img->height; i++) {
    free(img->row[i]);
  }
  free(img->row);
  free(img);

  *imgp = NULL;
}

/// Printing on the console

/// Output the raw BW image
void ImageRAWPrint(const Image img) {
  assert(img != NULL);

  printf("width = %d height = %d\n", img->width, img->height);
  printf("RAW image:\n");

  // Print the pixels of each image row
  for (uint32 i = 0; i < img->height; i++) {
    // The value of the first pixel in the current row
    int pixel_value = img->row[i][0];
    for (uint32 j = 1; img->row[i][j] != EOR; j++) {
      // Print the current run of pixels
      for (int k = 0; k < img->row[i][j]; k++) {
        printf("%d", pixel_value);
      }
      // Switch (XOR) to the pixel value for the next run, if any
      pixel_value ^= 1;
    }
    // At current row end
    printf("\n");
  }
  printf("\n");
}

/// Output the compressed RLE image
void ImageRLEPrint(const Image img) {
  assert(img != NULL);

  printf("width = %d height = %d\n", img->width, img->height);
  printf("RLE encoding:\n");

  // Print the compressed rows information
  for (uint32 i = 0; i < img->height; i++) {
    uint32 j;
    for (j = 0; img->row[i][j] != EOR; j++) {
      printf("%d ", img->row[i][j]);
    }
    printf("%d\n", img->row[i][j]);
  }
  printf("\n");
}

/// PBM BW file operations

// See PBM format specification: http://netpbm.sourceforge.net/doc/pbm.html

// Auxiliary function
static void unpackBits(int nbytes, const uint8 bytes[], uint8 raw_row[]) {
  // bitmask starts at top bit
  int offset = 0;
  uint8 mask = 1 << (7 - offset);
  while (offset < 8) {  // or (mask > 0)
    for (int b = 0; b < nbytes; b++) {
      raw_row[8 * b + offset] = (bytes[b] & mask) != 0;
    }
    mask >>= 1;
    offset++;
  }
}

// Auxiliary function
static void packBits(int nbytes, uint8 bytes[], const uint8 raw_row[]) {
  // bitmask starts at top bit
  int offset = 0;
  uint8 mask = 1 << (7 - offset);
  while (offset < 8) {  // or (mask > 0)
    for (int b = 0; b < nbytes; b++) {
      if (offset == 0) bytes[b] = 0;
      bytes[b] |= raw_row[8 * b + offset] ? mask : 0;
    }
    mask >>= 1;
    offset++;
  }
}

// Match and skip 0 or more comment lines in file f.
// Comments start with a # and continue until the end-of-line, inclusive.
// Returns the number of comments skipped.
static int skipComments(FILE* f) {
  char c;
  int i = 0;
  while (fscanf(f, "#%*[^\n]%c", &c) == 1 && c == '\n') {
    i++;
  }
  return i;
}

/// Load a raw PBM file.
/// Only binary PBM files are accepted.
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageLoad(const char* filename) {  ///
  int w, h;
  char c;
  FILE* f = NULL;
  Image img = NULL;

  check((f = fopen(filename, "rb")) != NULL, "Open failed");
  // Parse PBM header
  check(fscanf(f, "P%c ", &c) == 1 && c == '4', "Invalid file format");
  skipComments(f);
  check(fscanf(f, "%d ", &w) == 1 && w >= 0, "Invalid width");
  skipComments(f);
  check(fscanf(f, "%d", &h) == 1 && h >= 0, "Invalid height");
  check(fscanf(f, "%c", &c) == 1 && isspace(c), "Whitespace expected");

  // Allocate image
  img = AllocateImageHeader(w, h);

  // Read pixels
  int nbytes = (w + 8 - 1) / 8;  // number of bytes for each row
  // using VLAs...
  uint8 bytes[nbytes];
  uint8 raw_row[nbytes * 8];
  for (uint32 i = 0; i < img->height; i++) {
    check(fread(bytes, sizeof(uint8), nbytes, f) == (size_t)nbytes,
          "Reading pixels");
    unpackBits(nbytes, bytes, raw_row);
    img->row[i] = CompressRow(w, raw_row);
  }

  fclose(f);
  return img;
}

/// Save image to PBM file.
/// On success, returns unspecified integer. (No need to check!)
/// On failure, does not return, EXITS program!
int ImageSave(const Image img, const char* filename) {  ///
  assert(img != NULL);
  int w = img->width;
  int h = img->height;
  FILE* f = NULL;

  check((f = fopen(filename, "wb")) != NULL, "Open failed");
  check(fprintf(f, "P4\n%d %d\n", w, h) > 0, "Writing header failed");

  // Write pixels
  int nbytes = (w + 8 - 1) / 8;  // number of bytes for each row
  // using VLAs...
  uint8 bytes[nbytes];
  // unit8 raw_row[nbytes*8];
  for (uint32 i = 0; i < img->height; i++) {
    // UncompressRow...
    uint8* raw_row = UncompressRow(nbytes * 8, img->row[i]);
    // Fill padding pixels with WHITE
    memset(raw_row + w, WHITE, nbytes * 8 - w);
    packBits(nbytes, bytes, raw_row);
    size_t written = fwrite(bytes, sizeof(uint8), nbytes, f);
    check(written == (size_t)nbytes, "Writing pixels failed");
    free(raw_row);
  }

  // Cleanup
  fclose(f);
  return 0;
}

/// Information queries

/// Get image width
int ImageWidth(const Image img) {
  assert(img != NULL);
  return img->width;
}

/// Get image height
int ImageHeight(const Image img) {
  assert(img != NULL);
  return img->height;
}

/// Image comparison

int ImageIsEqual(const Image img1, const Image img2) {
  assert(img1 != NULL && img2 != NULL);

  // COMPLETE THE CODE
    assert(img1 != NULL && img2 != NULL);

    // Verifica se as dimensões das imagens são iguais
    if (ImageWidth(img1) != ImageWidth(img2) || ImageHeight(img1) != ImageHeight(img2)) {
        return 0;  // As imagens não são iguais
    }

    // Compara linha por linha
    for (uint32 i = 0; i < ImageHeight(img1); i++) {
        const int* row1 = img1->row[i];
        const int* row2 = img2->row[i];

        // Compara todos os elementos até o EOR
        uint32 j = 0;
        while (row1[j] != EOR && row2[j] != EOR) {
            if (row1[j] != row2[j]) return 0;
            j++;
        }

        // Verifica se ambos chegaram ao EOR ao mesmo tempo
        if (row1[j] != EOR || row2[j] != EOR) return 0;
    }

    return 1;  // As imagens são iguais
}

int ImageIsDifferent(const Image img1, const Image img2) {
  assert(img1 != NULL && img2 != NULL);
  return !ImageIsEqual(img1, img2);
}

/// Boolean Operations on image pixels

/// These functions apply boolean operations to images,
/// returning a new image as a result.
///
/// Operand images are left untouched and must be of the same size.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)

Image ImageNEG(const Image img) {
  assert(img != NULL);

  uint32 width = img->width;
  uint32 height = img->height;

  Image newImage = AllocateImageHeader(width, height);

  // Directly copying the rows, one by one
  // And changing the value of row[i][0]

  for (uint32 i = 0; i < height; i++) {
    uint32 num_elems = GetSizeRLERowArray(img->row[i]);
    newImage->row[i] = AllocateRLERowArray(num_elems);
    memcpy(newImage->row[i], img->row[i], num_elems * sizeof(int));
    newImage->row[i][0] ^= 1;  // Just negate the value of the first pixel run
  }

  return newImage;
}
//usa RLE

Image ImageAND(const Image img1, const Image img2) {
    // Instrumentação: medir tempo de execução
    double start_time = cpu_time();
    InstrCount[0] = 0;  // Contador para acessos à memória
    InstrCount[1] = 0;  // Contador para operações AND

    // Verificar se ambas as imagens são válidas
    assert(img1 != NULL && img2 != NULL);

    // Obter dimensões das imagens
    uint32 largura = img1->width;
    uint32 altura = img1->height;

    // Reservar memória para a imagem resultante
    Image resultado = AllocateImageHeader(largura, altura);
    if (resultado == NULL) {
        return NULL; // Falha na alocação
    }

    // Processar cada linha diretamente no formato RLE
    for (uint32 linha = 0; linha < altura; linha++) {
        const int* row1 = img1->row[linha];
        const int* row2 = img2->row[linha];

        // Reservar um array suficientemente grande para a linha resultante
        int* resultadoRow = AllocateRLERowArray(largura + 2);
        int index1 = 1, index2 = 1, indexRes = 1; // Começa no índice 1 (comprimentos)
        int cor1 = row1[0]; // Cor inicial da primeira imagem
        int cor2 = row2[0]; // Cor inicial da segunda imagem
        int corRes = cor1 & cor2; // Cor inicial do resultado

        int comprimento1 = row1[index1];
        int comprimento2 = row2[index2];

        resultadoRow[0] = corRes; // Definir a cor inicial do resultado

        while (row1[index1] != EOR && row2[index2] != EOR) {
            // Determinar o comprimento mínimo entre os dois runs
            int comprimentoMin = (comprimento1 < comprimento2) ? comprimento1 : comprimento2;

            // Adicionar um novo run ao resultado se a cor mudou
            if (indexRes == 1 || corRes != (cor1 & cor2)) {
                resultadoRow[indexRes++] = comprimentoMin;
                corRes = cor1 & cor2;
            } else {
                // Extender o comprimento do run atual
                resultadoRow[indexRes - 1] += comprimentoMin;
            }
            InstrCount[1]++; // Contar operação AND
            InstrCount[0] += 2; // Contar acessos às linhas RLE
            // Atualizar os comprimentos restantes
            comprimento1 -= comprimentoMin;
            comprimento2 -= comprimentoMin;
            // Avançar para o próximo run se o comprimento restante for zero
            if (comprimento1 == 0) {
                index1++;
                comprimento1 = row1[index1];
                cor1 ^= 1; // Alternar cor
            }
            if (comprimento2 == 0) {
                index2++;
                comprimento2 = row2[index2];
                cor2 ^= 1; // Alternar cor
            }
        }
        // Finalizar a linha resultante com EOR
        resultadoRow[indexRes++] = EOR;
        // Reservar a linha resultante na imagem
        resultado->row[linha] = AllocateRLERowArray(indexRes);
        memcpy(resultado->row[linha], resultadoRow, indexRes * sizeof(int));
        free(resultadoRow); // Libertar memória temporária
    }

    // Instrumentação: tempo de execução
    double end_time = cpu_time();
    printf("Tempo de execução da função ImageAND: %.6f segundos\n", end_time - start_time);
    InstrPrint();

    return resultado; // Retornar a imagem resultante
}


/*
Image ImageAND(const Image img1, const Image img2) {
    // Instrumentação: medir tempo de execução
    double start_time = cpu_time();
    InstrCount[0] = 0;  // Contador para acessos à memória
    InstrCount[1] = 0;  // Contador para operações AND
    // Verificar se ambas as imagens são válidas
    assert(img1 != NULL && img2 != NULL);
    // Obter dimensões das imagens
    uint32 largura = img1->width;
    uint32 altura = img1->height;
    // Reservar memória para a imagem resultante
    Image resultado = AllocateImageHeader(largura, altura);
    if (resultado == NULL) {
        return NULL; // Falha na alocação
    }
    // Processar cada linha das imagens
    for (uint32 linha = 0; linha < altura; linha++) {
        // Descompactar as linhas das duas imagens
        uint8* linha1 = UncompressRow(largura, img1->row[linha]);
        uint8* linha2 = UncompressRow(largura, img2->row[linha]);
        InstrCount[0] += 2 * largura; // Contar acessos às linhas descompactadas
        // Reservar memória para a linha resultante
        uint8* linhaResultado = malloc(largura * sizeof(uint8));
        InstrCount[0] += largura; // Contar alocação de memória para a linha resultante
        if (linhaResultado == NULL) {
            // Liberta memória em caso de falha
            free(linha1);
            free(linha2);
            ImageDestroy(&resultado);
            return NULL;
        }
        // Realizar a operação AND para cada pixel
        for (uint32 coluna = 0; coluna < largura; coluna++) {
            linhaResultado[coluna] = linha1[coluna] & linha2[coluna];
            InstrCount[1]++; // Contar operação AND
        }
        // Compactar a linha resultante e armazená-la na imagem final
        resultado->row[linha] = CompressRow(largura, linhaResultado);
        free(linhaResultado); // Libertar memória temporária da linha resultante
        InstrCount[0]++; // Contar libertação de memória
        free(linha1);
        free(linha2);
    }
    // Instrumentação: tempo de execução
    double end_time = cpu_time();
    printf("Tempo de execução da função ImageAND: %.6f segundos\n", end_time - start_time);
    InstrPrint();

    return resultado; // Retornar a imagem resultante
}
*/



Image ImageOR(const Image img1, const Image img2) {
  assert(img1 != NULL && img2 != NULL);

  // COMPLETE THE CODE
  // You might consider using the UncompressRow and CompressRow auxiliary files
  // Or devise a more efficient alternative
  assert(img1 != NULL && img2 != NULL);

  uint32 width = (img1->width < img2->width) ? img1->width : img2->width;
    uint32 height = (img1->height < img2->height) ? img1->height : img2->height;

    // Reservar o cabeçalho para a nova imagem
    Image result = AllocateImageHeader(width, height);
    if (result == NULL) {
        return NULL;
    }

    for (uint32 i = 0; i < height; i++) {
        // Descomprime as linhas das duas imagens até o menor tamanho
        uint8* row1 = UncompressRow(width, img1->row[i]);
        uint8* row2 = UncompressRow(width, img2->row[i]);

        if (row1 == NULL || row2 == NULL) {
            ImageDestroy(&result);
            if (row1) free(row1);
            if (row2) free(row2);
            return NULL;
        }

        // Aplica a operação OR diretamente
        for (uint32 j = 0; j < width; j++) {
            row1[j] |= row2[j]; // Modifica diretamente a linha descomprimida
        }

        // Comprime a linha resultante e armazena na nova imagem
        result->row[i] = CompressRow(width, row1);

        // Liberta memória das linhas temporárias
        free(row1);
        free(row2);
    }

    return result;
}

Image ImageXOR(Image img1, Image img2) {
  assert(img1 != NULL && img2 != NULL);

  // COMPLETE THE CODE
  // You might consider using the UncompressRow and CompressRow auxiliary files
  // Or devise a more efficient alternative
  uint32 width = (img1->width < img2->width) ? img1->width : img2->width;
    uint32 height = (img1->height < img2->height) ? img1->height : img2->height;
    // Aloca o cabeçalho da nova imagem
    Image result = AllocateImageHeader(width, height);
    if (result == NULL) {
        return NULL;
    }
    for (uint32 i = 0; i < height; i++) {
        // Descomprime as linhas das duas imagens
        uint8* row1 = UncompressRow(width, img1->row[i]);
        uint8* row2 = UncompressRow(width, img2->row[i]);

        if (row1 == NULL || row2 == NULL) {
            ImageDestroy(&result);
            if (row1) free(row1);
            if (row2) free(row2);
            return NULL;
        }
        // Aplica a operação XOR diretamente em row1
        for (uint32 j = 0; j < width; j++) {
            row1[j] ^= row2[j]; // Modifica diretamente row1
        }
        // Comprime a linha resultante e armazena na nova imagem
        result->row[i] = CompressRow(width, row1);
        // Liberta as linhas temporárias
        free(row1);
        free(row2);
    }
    return result;
}

/// Geometric transformations

/// These functions apply geometric transformations to an image,
/// returning a new image as a result.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)

/// Mirror an image = flip top-bottom.
/// Returns a mirrored version of the image.
/// Ensures: The original img is not modified.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageHorizontalMirror(const Image img) {
  assert(img != NULL);

  uint32 width = img->width;
  uint32 height = img->height;

  Image newImage = AllocateImageHeader(width, height);

  // COMPLETE THE CODE
  if (newImage == NULL) return NULL;

    // Copia as linhas de forma invertida
    for (uint32 i = 0; i < height; i++) {
        uint32 invertedIndex = height - 1 - i;
        uint32 numElems = GetSizeRLERowArray(img->row[invertedIndex]);
        newImage->row[i] = AllocateRLERowArray(numElems);
        memcpy(newImage->row[i], img->row[invertedIndex], numElems * sizeof(int));
    }

    return newImage;
}

/// Mirror an image = flip left-right.
/// Returns a mirrored version of the image.
/// Ensures: The original img is not modified.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageVerticalMirror(const Image img) {
  assert(img != NULL);

  uint32 width = img->width;
  uint32 height = img->height;

  Image newImage = AllocateImageHeader(width, height);

  // COMPLETE THE CODE
  if (newImage == NULL) return NULL;

    for (uint32 i = 0; i < height; i++) {
        uint8* uncompressedRow = UncompressRow(width, img->row[i]);
        uint8* mirroredRow = malloc(width * sizeof(uint8));
        check(mirroredRow != NULL, "malloc");

        // Inverte os pixeis da linha
        for (uint32 j = 0; j < width; j++) {
            mirroredRow[j] = uncompressedRow[width - 1 - j];
        }

        newImage->row[i] = CompressRow(width, mirroredRow);

        free(uncompressedRow);
        free(mirroredRow);
    }

    return newImage;
}

/// Replicate img2 at the bottom of imag1, creating a larger image
/// Requires: the width of the two images must be the same.
/// Returns the new larger image.
/// Ensures: The original images are not modified.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageReplicateAtBottom(const Image img1, const Image img2) {
  assert(img1 != NULL && img2 != NULL);
  assert(img1->width == img2->width);

  uint32 new_width = img1->width;
  uint32 new_height = img1->height + img2->height;

  Image newImage = AllocateImageHeader(new_width, new_height);

  // COMPLETE THE CODE
  if (newImage == NULL) return NULL;

    // Copia as linhas da primeira imagem
    for (uint32 i = 0; i < img1->height; i++) {
        uint32 numElems = GetSizeRLERowArray(img1->row[i]);
        newImage->row[i] = AllocateRLERowArray(numElems);
        memcpy(newImage->row[i], img1->row[i], numElems * sizeof(int));
    }

    // Copia as linhas da segunda imagem
    for (uint32 i = 0; i < img2->height; i++) {
        uint32 numElems = GetSizeRLERowArray(img2->row[i]);
        newImage->row[img1->height + i] = AllocateRLERowArray(numElems);
        memcpy(newImage->row[img1->height + i], img2->row[i], numElems * sizeof(int));
    }

    return newImage;
}

/// Replicate img2 to the right of imag1, creating a larger image
/// Requires: the height of the two images must be the same.
/// Returns the new larger image.
/// Ensures: The original images are not modified.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
Image ImageReplicateAtRight(const Image img1, const Image img2) {
  assert(img1 != NULL && img2 != NULL);
  assert(img1->height == img2->height);

  uint32 new_width = img1->width + img2->width;
  uint32 new_height = img1->height;

  Image newImage = AllocateImageHeader(new_width, new_height);

  // COMPLETE THE CODE
for (uint32 i = 0; i < new_height; i++) {
    uint8* row1 = UncompressRow(img1->width, img1->row[i]);
    uint8* row2 = UncompressRow(img2->width, img2->row[i]);
    uint8* combinedRow = malloc(new_width * sizeof(uint8));
    check(combinedRow != NULL, "malloc");
    // Junta as linhas descomprimidas
    memcpy(combinedRow, row1, img1->width * sizeof(uint8));
    memcpy(combinedRow + img1->width, row2, img2->width * sizeof(uint8));
    newImage->row[i] = CompressRow(new_width, combinedRow);
    free(row1);
    free(row2);
    free(combinedRow);
}

return newImage;

}
