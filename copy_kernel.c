#include <string.h>
#include <stdlib.h>
#include "copy_kernel.h"


void bandwidth(dout_t *output_port, unsigned char *input_port)
{
    #pragma HLS INTERFACE m_axi port=output_port offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=input_port offset=slave bundle=gmem1

    #pragma HLS INTERFACE s_axilite port=output_port bundle=control
    #pragma HLS INTERFACE s_axilite port=input_port bundle=control

    #pragma HLS INTERFACE s_axilite port=return bundle=control

//    #pragma HLS DATA_PACK variable=output_port

	unsigned long blockindex;
	unsigned int i;

    outerloop:
    for (blockindex = 0; blockindex < DATA_SIZE; blockindex++) {
        #pragma HLS pipeline ii=1
        unsigned char loadAB = input_port[blockindex];
        unsigned char resultAB = 0xFA + loadAB;
        if (resultAB != 0xFA) {
            interloop:
            for (i = 0; i < MULT; i++) {
            #pragma HLS unroll
                output_port[blockindex].data[i] = resultAB;
            }
        }
    }
}
