#include <string.h>
#include <stdlib.h>
#include "copy_kernel.h"


void bandwidth(dout_t *output_port, din_t *input_port)
{
    #pragma HLS INTERFACE m_axi port=output_port offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=input_port offset=slave bundle=gmem1

    #pragma HLS INTERFACE s_axilite port=output_port bundle=control
    #pragma HLS INTERFACE s_axilite port=input_port bundle=control

    #pragma HLS INTERFACE s_axilite port=return bundle=control

    #pragma HLS DATA_PACK variable=input_port
    #pragma HLS DATA_PACK variable=output_port

	unsigned long blockindex;
	unsigned int i;

	outerloop:
	for (blockindex = 0; blockindex < DATA_SIZE/64; blockindex++) {
		din_t inbuffer = input_port[blockindex];
#pragma HLS ARRAY_PARTITION variable=inbuffer complete
		dout_t outbuffer;
#pragma HLS ARRAY_PARTITION variable=outbuffer complete
		innerloop:// read two bytes at a time
		for (i = 0; i < 64; i+=2) {
#pragma HLS pipeline ii=1 // ensure that the iteration interval is just 1 cycle
			unsigned char loadAB = inbuffer.data[i]; // look for "AB" in generated RTL
			unsigned char loadCD = inbuffer.data[i+1]; // look for "CD" in generated RTL
			unsigned char resultAB = 0xFA + loadAB;
			unsigned char resultCD = 0xFB + loadCD;

			outbuffer.data[i] = resultAB;
			outbuffer.data[i+1] = resultCD;
		}

		output_port[blockindex] = outbuffer; // commit reports
	}

}
