#include <string.h>
#include <stdlib.h>
#include "copy_kernel.h"

#define COUNT 64
#define WHICH 2

void bandwidth(wwrd_512 *output_port, wwrd_256 *input_port)
{
    #pragma HLS INTERFACE m_axi port=output_port offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=input_port offset=slave bundle=gmem1

    #pragma HLS INTERFACE s_axilite port=output_port bundle=control
    #pragma HLS INTERFACE s_axilite port=input_port bundle=control

    #pragma HLS INTERFACE s_axilite port=return bundle=control

    #pragma HLS DATA_PACK variable=input_port
    #pragma HLS DATA_PACK variable=output_port

#if WHICH == 0
    unsigned long blockindex;
    unsigned int i;

    outerloop:
    for (blockindex=0; blockindex < DATA_SIZE/64; blockindex+=2) {
        wwrd_512 reports;
#pragma HLS DATA_PACK variable=reports
#pragma HLS RESOURCE variable=reports core=RAM_2P_BRAM
        wwrd_256 inbuffer = input_port[blockindex];
#pragma HLS DATA_PACK variable=inbuffer
#pragma HLS RESOURCE variable=inbuffer core=RAM_2P_BRAM

        innerloop:
        for (i = 0; i < 64; i++) {
#pragma HLS PIPELINE ii=1
			reports.data[i] = (unsigned short) inbuffer.data[i];

        }

        output_port[blockindex] = reports;
    }
#endif

#if WHICH ==1
	unsigned long blockindex;
	unsigned int i, j;

	outerloop:
	for (blockindex = 0; blockindex < DATA_SIZE/32; blockindex += COUNT) {
		wwrd_256 inbuffer[COUNT];
#pragma HLS RESOURCE variable=inbuffer core=RAM_2P_BRAM latency=1
		wwrd_512 outbuffer[COUNT];
#pragma HLS RESOURCE variable=outbuffer core=RAM_2P_BRAM latency=1

		// populate the inbuffer
		readloop: for (i = 0; i < COUNT; i++) {
#pragma HLS PIPELINE

			inbuffer[i] = input_port[blockindex+i];
		}

		// compute
		computeloop: for (i = 0; i < COUNT; i++) {
			for (j = 0; j < 32; j++) {
#pragma HLS pipeline ii=1
#pragma HLS latency max=1
				outbuffer[i].data[j] = (unsigned short) inbuffer[i].data[j];
			}
		}

		// write
		writeloop: for (i = 0; i < COUNT; i++) {
#pragma HLS PIPELINE

			output_port[blockindex+i] = outbuffer[i];
		}
	}
#endif


#if WHICH==2
	unsigned long blockindex;
	unsigned int i;

	outerloop:
	for (blockindex = 0; blockindex < DATA_SIZE/32; blockindex++) {
		wwrd_256 inbuffer = input_port[blockindex];
#pragma HLS DATA_PACK variable=inbuffer
		wwrd_512 outbuffer;
#pragma HLS DATA_PACK variable=outbuffer
		innerloop:// read two bytes at a time
		for (i = 0; i < 32; i+=2) {
#pragma HLS pipeline ii=1
			unsigned char load0AB = inbuffer.data[i];
			unsigned char load1CD = inbuffer.data[i+1];

			unsigned short result0 = 0xAB00 + load0AB;
			unsigned short result1 = 0xCD00 + load1CD;

			outbuffer.data[i] = result0;
			outbuffer.data[i+1] = result1;
		}

		output_port[blockindex] = outbuffer;
	}
#endif

}
