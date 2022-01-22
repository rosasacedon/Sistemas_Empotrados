#include <stdlib.h>
#include <stdio.h>

#include "img_filter.h"

#define INPUT_IMAGE "./molinos.bmp"
#define OUTPUT_IMAGE "./molinos_out.bmp"

#define BMP_MAX_HEADER_SIZE 1024
#define BMP_HEADER_SIZE_OFFSET 10

unsigned char bmp_header[BMP_MAX_HEADER_SIZE];
unsigned long offset;

int leer_bmp_rgb_data(const char *filename,STREAM_IN &input) {
	FILE *fp;

	fp = fopen(filename,"rb");
	if (!fp) {
		printf("Error al abrir el archivo.\n");
		return -1;
	}

	//Posicionar el puntero de lectura al campo de longitud
	fseek(fp,BMP_HEADER_SIZE_OFFSET,SEEK_SET);

	unsigned char buffer[4];
	fread(buffer,1,4,fp);

	//LSB - Less significant byte comes first
	offset = buffer[3]*256*256*256 + buffer[2]*256*256 + buffer[1]*256 + buffer[0];

	fseek(fp,0,SEEK_END);

	unsigned long length = ftell(fp);

	printf("%02X %02X %02X %02X\nOffset %ld\nLength: %ld\n",
			buffer[0],buffer[1],buffer[2],buffer[3],offset,length);

	if (offset > BMP_MAX_HEADER_SIZE) {
		printf("El tamaño de la cabecera BMP es mayor del permitido: %d\n",offset);
		return -1;
	}
	fseek(fp,0,SEEK_SET);
	//Leer cabecera para guardar resultado
	fread(bmp_header,1,offset,fp);

	fseek(fp,offset,SEEK_SET);

	for (int datap = 0; datap < length-offset; datap += 3) {
		if (fread(buffer,1,3,fp) != 3) {
			printf("Cuidado! Has leído menos datos de los que deberías.\n");
		}
		pixel_rgb_t pixel_color;
		R(pixel_color) = buffer[0];
		G(pixel_color) = buffer[1];
		B(pixel_color) = buffer[2];
		input.write(pixel_color);
	}

	printf("Comprobando FEOF: %d\n",feof(fp));
	fclose(fp);

	return 0;
}

int escribir_bmp_gray_data(const char *filename,STREAM_OUT &output) {
	FILE *fp;

	fp = fopen(filename,"wb");
	if (!fp) {
		printf("Error al abrir el archivo.\n");
		return -1;
	}

	//Write header
	fwrite(bmp_header,1,offset,fp);

	//Write data
	do {
		pixel_gray_t pixel_gris = output.read();
		unsigned char p[3];
		p[0] = p[1] = p[2] = pixel_gris;
		fwrite(p,1,3,fp);
	} while (output.size());

	printf("Posición del puntero buffer de escritura: %ld\n",ftell(fp));
	fclose(fp);
	return 0;
}


int main (int argc, char** argv)
{

	STREAM_IN img_in("InputStream");
	STREAM_OUT img_out("OutputStream");
	AXI_STREAM_IN axi_in;
	AXI_STREAM_OUT axi_out;
	unsigned short row, col;

	if (leer_bmp_rgb_data(INPUT_IMAGE,img_in))
		return -1;

	for (row = 0; row < ROWS; row++) {
		for (col = 0; col < COLS; col++) {
			AXI_VAL_IN e;
			e.data = img_in.read();
			e.strb = -1;
			e.keep = -1; //e.strb;
			e.user = 1;
			e.last = ((col == (COLS-1)) && (row == (ROWS-1)));
			e.id = 1;
			e.dest = 1;
			axi_in.write(e);
		}
	}

	printf("Size of input stream %d\n",img_in.size());
	//Función top
	img_pipeline(axi_in,axi_out,50);


	for (row = 0; row < ROWS; row++) {
		for (col = 0; col < COLS; col++) {
			AXI_VAL_OUT e;
			e = axi_out.read();
			img_out.write(e.data);
			//printf("Debug: %d ",e.last.to_int());
		}
	}

	printf("Size of input stream %d\n",img_in.size());
	printf("Size of output stream %d\n",img_out.size());

	escribir_bmp_gray_data(OUTPUT_IMAGE,img_out);
	printf("Size of output stream %d\n",img_out.size());

	printf("Exit...\n");

	return 0;
}

