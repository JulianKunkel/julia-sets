all: juliaMengen.c write-bmp.c
	gcc -Wall -O3 juliaMengen.c write-bmp.c -o juliaMengen -std=gnu99 -DUSE_BMP -lrt 
	#-DCOLORSCHEMA

clean:
	rm juliaMengen
