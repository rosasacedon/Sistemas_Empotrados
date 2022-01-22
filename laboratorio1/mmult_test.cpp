#include <stdio.h>
#include <stdlib.h>

#include "mmult.h"

void convert2array(WORD_MEM a, T out[NUM_ELEMS_WORD]);
void convert2word(WORD_MEM &a, T out[NUM_ELEMS_WORD]);

void mmult_sw(T a[DIM][DIM], T b[DIM][DIM], T out[DIM][DIM])
{
	// matrix multiplication of a A*B matrix
	for (int ia = 0; ia < DIM; ++ia)
		for (int ib = 0; ib < DIM; ++ib)
		{

			float sum = 0;

			for (int id = 0; id < DIM; ++id)

				sum += a[ia][id] * b[id][ib];

			out[ia][ib] = sum;
		}
}


int main_standalone(void)
{

	int ret_val = 0;

	int i,j, err;

	T matOp1[DIM][DIM];
	T matOp2[DIM][DIM];
	T matMult_sw[DIM][DIM];
	T matMult_hw[DIM][DIM];

	/** Matrix Initiation */
	for(i = 0; i<DIM; i++)
		for(j = 0; j<DIM; j++)
			matOp1[i][j] = (float)(i+j);

	for(i = 0; i<DIM; i++)
		for(j = 0; j<DIM; j++)
			matOp2[i][j] = (float)(i*j);
	/** End of Initiation */

	printf("NORMAL MODE\r\n");
	mmult_hw(matOp1, matOp2, matMult_hw);

	/* reference Matrix Multiplication */
	mmult_sw(matOp1, matOp2, matMult_sw);

	/** Matrix comparison */
	err = 0;
	for (i = 0; (i<DIM && !err); i++)
		for (j = 0; (j<DIM && !err); j++)
			if (matMult_sw[i][j] != matMult_hw[i][j])
				err++;

	if (err == 0)
		printf("Matrixes identical ... Test successful!\r\n");
	else
		printf("Test failed!\r\n");

	return err;

}


/* Test mmult_hw_wrapped function */
int main_axi(void)
{
	int i,j, err;

	ap_uint<U> user;
	ap_uint<TI> id;
	ap_uint<TD> dest;


	T matOp1[DIM][DIM];
	T matOp2[DIM][DIM];
	T matMult_sw[DIM][DIM];
	T matMult_hw[DIM][DIM];

	/** Matrix Initiation */
	for(i = 0; i<DIM; i++)
		for(j = 0; j<DIM; j++)
			matOp1[i][j] = (float)(i+j);

	for(i = 0; i<DIM; i++)
		for(j = 0; j<DIM; j++)
			matOp2[i][j] = (float)(i*j);
	/** End of Initiation */


	//printf("DEBUGGING AXI4 STREAMING DATA TYPES!\r\n");

	// prepare data for the DUT
	hls::stream<AXI_VAL> inp_stream("INSW");
	hls::stream<AXI_VAL> out_stream("OUTSW");

	AXI_VAL e;

	// stream in the first input  matrix
	for(int i=0; i<DIM; i++)
		for(int j=0; j<DIM; j+=NUM_ELEMS_WORD)
		{

			convert2word(e.data,&matOp1[i][j]);

			e.strb = -1;
			e.keep = 15; //e.strb;
			e.user = 0;
			e.last = 0;
			e.id = 1;
			e.dest = 2;

			inp_stream.write(e);

		}
	// stream in the second input  matrix
	for(int i=0; i<DIM; i++)
		for(int j=0; j<DIM; j+=NUM_ELEMS_WORD)
		{

			convert2word(e.data,&matOp2[i][j]);
			e.strb = -1;
			e.keep = 15; //e.strb;
			e.user = 0;
			e.last = ((i==DIM-1) && (j==(DIM-NUM_ELEMS_WORD)));
			e.id = 2;
			e.dest = 3;

			inp_stream.write(e);

		}


	//call the DUT
	mmult_hw_wrapped(inp_stream, out_stream);

	// extract the output matrix from the out stream
	for(int i=0; i<DIM; i++)
		for(int j=0; j<DIM; j+=NUM_ELEMS_WORD)
		{


			e = out_stream.read();
			convert2array(e.data,&matMult_hw[i][j]);

			//printf("AXIS Control signals: %d\n",e.last.to_bool());
		}


	/* reference Matrix Multiplication */
	mmult_sw(matOp1, matOp2, matMult_sw);

	/** Matrix comparison */
	err = 0;
	for (i = 0; (i<DIM); i++) {
		for (j = 0; (j<DIM); j++) {
			if (matMult_sw[i][j] != matMult_hw[i][j]) {
				err++;
				printf("Error (%d,%d) %f %f\r\n",i,j,matMult_sw[i][j],matMult_hw[i][j]);

			}
		}
	}
	if (err == 0)
		printf("Matrixes identical ... Test successful!\r\n");
	else
		printf("Test failed! (%d)\r\n",err);

	return err;
}

int main(void) {
	return main_standalone();
}


void convert2array(WORD_MEM w, T out[NUM_ELEMS_WORD]) {
	conv_t c;

	convert2array_label1:for (int k=0; k<NUM_ELEMS_WORD;k++) {

		c.in = w((k+1)*32-1,k*32);
		//
		out[k] = c.out;
	}

}
void convert2word(WORD_MEM &w, T in[NUM_ELEMS_WORD]) {
	conv_t c;
	WORD_MEM _w;

	convert2word_label0:for (int k=0; k<NUM_ELEMS_WORD;k++) {
		c.out = in[k];
		_w((k+1)*32-1,k*32)= c.in;
		//		_w((NUM_ELEMS_WORD-(k))*32-1,(NUM_ELEMS_WORD-(k+1))*32)= c.in;
	}
	w = _w;
}

