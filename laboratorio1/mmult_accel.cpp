#include <stdio.h>
#include <stdlib.h>

#include "mmult.h"



// --------------------------------------------------------
// function to be accelerated in HW. Standalone mode.

void mmult_hw(T a[DIM][DIM], T b[DIM][DIM], T out[DIM][DIM])
{
	// matrix multiplication of a A*B matrix
	L1:for (int ia = 0; ia < DIM; ++ia)
		L2:for (int ib = 0; ib < DIM; ++ib)
		{
			T sum = 0;

			L3:for (int id = 0; id < DIM; ++id) {
				sum += a[ia][id] * b[id][ib];
			}
			out[ia][ib] = sum;
		}

	return;
}



//void dataflow_func(T a[DIM][DIM], T b[DIM][DIM], T out[DIM][DIM], int ia, int ib) {
//	T _sum = 0;
//	hls::stream<T> sum_v;
//
//	L3:for (int id = 0; id < DIM; ++id) {
//		sum_v.write(a[ia][id] * b[id][ib]);
//	}
//
//	L4:for (int ic = 0; ic < DIM; ++ic) {
//		_sum += sum_v.read();
//	}
//	out[ia][ib] = _sum;
//
//}
//void mmult_hw(T a[DIM][DIM], T b[DIM][DIM], T out[DIM][DIM])
//{
//	T sum;
//	// matrix multiplication of a A*B matrix
//	L1:for (int ia = 0; ia < DIM; ++ia) {
//		L2:for (int ib = 0; ib < DIM; ++ib)
//		{
//			dataflow_func(a,b,out,ia,ib);
//
//		}
//
//	}
//
//	return;
//}



//
// --------------------------------------------------------------------
// function to be accelerated in HW wrapped with AXI4-Stream interface
void mmult_hw_wrapped (
		hls::stream<AXI_VAL> &INPUT_STREAM,
		hls::stream<AXI_VAL> &OUTPUT_STREAM)
{
#pragma HLS INTERFACE s_axilite port=return     bundle=CONTROL_BUS
#pragma HLS INTERFACE axis      port=INPUT_STREAM
#pragma HLS INTERFACE axis      port=OUTPUT_STREAM

	ap_uint<U> user;
	ap_uint<TI> id;
	ap_uint<TD> dest;


	T a[DIM][DIM];
	T b[DIM][DIM];
	T out[DIM][DIM];

	// stream in first matrix
	for(int i=0; i<DIM; i++)
		for(int j=0; j<DIM; j+= NUM_ELEMS_WORD)
		{
#pragma HLS PIPELINE II=1
			WORD_MEM w = INPUT_STREAM.read().data;
			conv_t c;
			for (int k=0; k<NUM_ELEMS_WORD;k++) {
				c.in = w((k+1)*32-1,k*32);
				a[i][j+k] = c.out;
			}
		}

	// stream in second matrix
	for(int i=0; i<DIM; i++)
		for(int j=0; j<DIM; j+= NUM_ELEMS_WORD)
		{
#pragma HLS PIPELINE II=1
			WORD_MEM w = INPUT_STREAM.read().data;
			conv_t c;
			for (int k=0; k<NUM_ELEMS_WORD;k++) {
				c.in = w((k+1)*32-1,k*32);
				b[i][j+k] = c.out;
			}

		}

	// do HW multiplication
	mmult_hw(a,b,out);

	// stream out result matrix
	for(int i=0; i<DIM; i++)
		for(int j=0; j<DIM; j+= NUM_ELEMS_WORD)
		{
#pragma HLS PIPELINE II=1
			AXI_VAL e;
			conv_t c;
			WORD_MEM w;
			for (int k=0; k<NUM_ELEMS_WORD;k++) {

				c.out = out[i][j+k];
				w((k+1)*32-1,k*32)= c.in;
			}
			e.data = w;
			e.strb = -1;
			e.keep = 15; //e.strb;
			e.user = U;
			e.last = ((i == (DIM-1)) && (j == (DIM-NUM_ELEMS_WORD)));
			e.id = TI;
			e.dest = TD;
			OUTPUT_STREAM.write(e);
		}

	return;

}
