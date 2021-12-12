/**
 * Julian's Code für Julia-Mengen - see http://de.wikipedia.org/wiki/Julia-Menge ;-)
 * (c) Julian Kunkel 2010
 *
 * Licensed under BSD License...
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>

#include <time.h>

// allows to specify size upon compile time:
// gcc -Wall juliaMengen.c -std=c99 -DSIZE=20
// post processing, i.e. convert to png with imagemagick:

// cool parameters:  ./juliaMengen -0.39 0.6 test.bmp 10 0 0 1000
// very long run: ./juliaMengen -0.39 0.5 test.bmp 0 0 0 10
//   convert test.bmp julia-0.39Plus0.6i.png


// complex data type:
typedef struct {
   double real;
   double complex;
} COMPLEX;

static void writeMatrix(int width, int height, int ** matrix, char * file, int maxIter, int colorSchema, int COLORMAX );

static inline void cAdd(COMPLEX * a, COMPLEX * b, COMPLEX * out){
  out->real = a->real + b->real ;
  out->complex = a->complex + b->complex;
}

// out and in must be different variables, otherwise an error occurs.
static inline void cMul(COMPLEX * a, COMPLEX * b, COMPLEX * out){
  out->real = a->real * b->real - a->complex * b->complex;
  out->complex = a->complex * b->real + a->real *  b->complex ;
}

static inline void cMulS(COMPLEX * a, double val, COMPLEX * out){
  out->real = a->real * val;
  out->complex = a->complex * val;
}

static inline void function(COMPLEX * a){
}

/*
  ./juliaMengen -0.6 0.6 test.bmp 13 0 0 5000
*/

int main(int argC, char ** argV){
    if (argC != 8){
	printf("Syntax: <Program> <Value of C (real)> <Value of C(complex part)> <ImageFileName> <ColorSchema> <x-offset> <y-offset> <size>\n");
	printf(" ColorSchema: <R|G|B> = 2 | 4 | 8  i.e. value of 10 is Purple\n");
	printf("              add 1 to use single color, otherwise multicolor\n");
	exit(1);
    }

    double cVal = atof(argV[1]);
    double cVali = atof(argV[2]);
    char * file = argV[3];
    int colorSchema = atoi(argV[4]);
    int colormax =  256;
    double x_offset = atof(argV[5]);
    double y_offset = atof(argV[6]);
    int size = atoi(argV[7]);

    if (colorSchema & 1) {
      colormax =  256;
    }else{
      colormax = (256 * 256 * 256);
    }

    if(colorSchema == 0){
      printf("Error: colorSchema 0 does not make sense, I will use 14 instead\n");
      colorSchema = 14;
    }

    printf("Julia-Menge %f + %fi into file %s\n", cVal, cVali, file);

    // matrix enthält anzahl der Iterationen bis Julia Menge divergiert.
    int * matrixData = malloc(sizeof(COMPLEX) * size * size);

    memset(matrixData, colormax, sizeof(COMPLEX) * size * size);

    // allocate memory for rows:
    int ** matrix = malloc(sizeof(void *) * size);

    // set rows:
    for (int y = 0 ; y < size; y++){
      matrix[y] = matrixData + size * y;
    }

    COMPLEX c = {cVal, cVali};
    COMPLEX tmp;

    struct timespec startTime;
    struct timespec endTime;

    int maxIter = 1;
    int have_one = 0;

    clock_gettime(CLOCK_REALTIME, & startTime);

    // now walk through every point in x/y-space:

    // compute JULIA
    for (int y = 0; y < size; y++){
	for (int x = 0; x < size; x++){

	int i;
	COMPLEX cur = {-1.0 + x_offset + 2.0 / size * x, -1.0 + y_offset + 2.0 / size * y};

	for (i = 0 ; i < colormax; i ++){
	  double abs = (cur.real * cur.real + cur.complex * cur.complex);

	  if( abs > 4 ){
	    // now we know it will explode!
	    matrix[x][y] = i;

	    if( maxIter < i) {
	      maxIter = i;
	    }

	    break;
	  }

	  function(& cur);
	  cMul(& cur, & cur, & tmp);
	  cAdd(& tmp, & c, & cur);
	}

	if(i == colormax){
	  have_one = 1;
	}
      }
    }
    // end computation

    clock_gettime(CLOCK_REALTIME, & endTime);

    printf("Time: %fs Max iterations %d - unlimited recursion: %d\n", endTime.tv_sec - startTime.tv_sec + (endTime.tv_nsec - startTime.tv_nsec) * 1e-9, maxIter, have_one);

    // write Matrix
    writeMatrix(size, size, matrix, file, maxIter, colorSchema, colormax);

    free(matrixData);

    return 0;
}



/*
 * The following code works and should not be modified...
 */
int write_bmp(const char *filename, int width, int height, char *rgb);

void writeMatrix( int width, int height, int ** matrix, char * file, int maxIter, int colorSchema, int colormax ){
  char * rgb = malloc(width*height*3);

  double part = ( colormax - 1.0 ) / maxIter;
  int colorMultiplier[] = { (colorSchema & 2) / 2, (colorSchema & 4) / 4, (colorSchema & 8) / 8 };

  // change matrix format and compute real values:
  for (int y = 0 ; y < height; y++){
    // much room for optimization here.
    for (int x = 0 ; x < width ; x++){
      int color = (int) (  matrix[x][y] * part - 0.01 );

      int pos = 3 * (y * width + x );

      if(  colorSchema & 1   ){
	assert(color >= 0 && color <= 255);

	rgb[pos] =  color *  colorMultiplier[0];
	rgb[pos + 1 ] =  color *  colorMultiplier[1];
	rgb[pos + 2 ] =  color *  colorMultiplier[2];
      }else{
	assert(color >= 0 && color <= colormax);
	#define shft(x) (( color & ( 1 << (x*3) )) >> (x *2) )
	rgb[pos] = shft(0)+ shft(1) + shft(2) + shft(3) + shft(4)+ shft(5)+ shft(6)+ shft(7) ;
	#undef shft
	#define shft(x) (( color & ( 1 << (x*3+1) )) >> (x *2 + 1) )
	rgb[pos + 1] = shft(0)+ shft(1) + shft(2) + shft(3) + shft(4)+ shft(5)+ shft(6)+ shft(7) ;
	#undef shft
	#define shft(x) (( color & ( 1 << (x*3+2) )) >> (x *2 + 2 ) )
	rgb[pos + 2] = shft(0)+ shft(1) + shft(2) + shft(3) + shft(4)+ shft(5)+ shft(6)+ shft(7) ;
	#undef shft
      }
    }
  }

  write_bmp(file, width, height, rgb);

  free(rgb);
}