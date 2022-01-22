#include "img_filter.h"


void rgb2gray(STREAM_IN& in,STREAM_OUT& img_gray) {

	unsigned short row,col;


	for (row = 0; row < ROWS; row++) {
		for (col = 0; col < COLS; col++) {
			pixel_rgb_t pixel_color;
			pixel_gray_t pixel_gray;
			pixel_gray_t pixel_negative;

			pixel_color = in.read();

			//R, G y B son macros que toman una parte de los bits
			pixel_gray = (R(pixel_color) + G(pixel_color) + B(pixel_color))/3;
			// Pasamos a negativo el pixel
			//pixel_negative = 255 - pixel_gray;

			img_gray.write(pixel_gray);
		}
	}
}

void threshold(hls::stream<pixel_gray_t> &img_gray,STREAM_OUT& out,unsigned int thres_value) {
	unsigned short row,col;

	for (row = 0; row < ROWS; row++) {
		for (col = 0; col < COLS; col++) {

			pixel_gray_t pixel_gray;
			pixel_gray = img_gray.read();
			if (pixel_gray < thres_value) {
				pixel_gray = 0;
			}
			out.write(pixel_gray);

		}
	}

}

void read_img_input(AXI_STREAM_IN &in,STREAM_IN& out) {
	unsigned short row,col;

	for (row = 0; row < ROWS; row++) {
		for (col = 0; col < COLS; col++) {
			out.write(in.read().data);
		}
	}
}

void write_img_out(STREAM_OUT &in,AXI_STREAM_OUT& out, unsigned int scal_factor) {
	unsigned short row,col;

	for (row = 0; row < ROWS; row++) {
		for (col = 0; col < COLS; col++) {
			if(((row % scal_factor) == 0) && ((col % scal_factor) == 0)){
				AXI_VAL_OUT e;
				e.data = in.read();
				e.strb = -1;
				e.keep = -1; //e.strb;
				e.user = 1;
				e.last = ((col == (COLS-1)) && (row == (ROWS-1)));
				e.id = 1;
				e.dest = 1;
				out.write(e);
			}
		}
	}
}

void negative(hls::stream<pixel_gray_t>& in,hls::stream<pixel_gray_t>& out){
	unsigned short row,col;


		for (row = 0; row < ROWS; row++) {
			for (col = 0; col < COLS; col++) {
				pixel_gray_t pixel_gray;
				pixel_gray_t pixel_negative;

				pixel_gray = in.read();

				// Pasamos a negativo el pixel
				pixel_negative = 255 - pixel_gray;

				out.write(pixel_negative);
			}
		}
}
void reescalado(hls::stream<pixel_gray_t> &in,STREAM_OUT& out, unsigned int scal_factor){
	unsigned short row,col;

	for (row = 0; row < ROWS; row++) {
		for (col = 0; col < COLS; col++) {
			pixel_gray_t pixel_negative;

			pixel_negative = in.read();
			if(((row % scal_factor) == 0) && ((col % scal_factor) == 0))
				out.write(pixel_negative);
		}
	}
}

void img_pipeline(AXI_STREAM_IN& in, AXI_STREAM_OUT& out,unsigned int thres_value, unsigned int scal_factor) {
#pragma HLS INTERFACE mode=s_axilite bundle=control port=return
#pragma HLS INTERFACE mode=s_axilite bundle=control port=thres_value
#pragma HLS INTERFACE mode=s_axilite bundle=control port=scal_factor
#pragma HLS INTERFACE mode=axis port=out
#pragma HLS INTERFACE mode=axis port=in

#pragma HLS dataflow

	hls::stream<pixel_gray_t> img_grayscale("GRAY"),img_thres("THRES");
	hls::stream<pixel_rgb_t> img_color("COLOR");
	hls::stream<pixel_gray_t> img_negative("NEGATIVE");
	hls::stream<pixel_gray_t> reescal("REESCAL");


	read_img_input(in,img_color);
	rgb2gray(img_color,img_grayscale);
	threshold(img_grayscale,img_thres,thres_value);
	negative(img_thres, img_negative);
	reescalado(img_negative,reescal,scal_factor);
	write_img_out(reescal,out,scal_factor);

	return;
}
