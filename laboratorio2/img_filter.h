#ifndef _IMG_FILTER_H_
#define _IMG_FILTER_H_

#include <hls_stream.h>
#include <ap_int.h>
#include <ap_axi_sdata.h>



#define ROWS 720
#define COLS 1280

typedef ap_uint<8> pixel_gray_t;
typedef ap_uint<24> pixel_rgb_t;

typedef hls::stream<pixel_rgb_t> STREAM_IN;
typedef hls::stream<pixel_gray_t> STREAM_OUT;

typedef ap_axiu<24,1,1,1> AXI_VAL_IN;
typedef hls::stream<AXI_VAL_IN> AXI_STREAM_IN;

typedef ap_axiu<8,1,1,1> AXI_VAL_OUT;
typedef hls::stream<AXI_VAL_OUT> AXI_STREAM_OUT;


#define R(p) (p(23,16))
#define G(p) (p(15,8))
#define B(p) (p(7,0))



//void img_pipeline(STREAM_IN& in, STREAM_OUT& out,unsigned int thres_value);
void img_pipeline(AXI_STREAM_IN& in, AXI_STREAM_OUT& out,unsigned int thres_value);


#endif
