#include <string.h>
#include <stdlib.h>

#define WORD_SIZE 64//512 bits

typedef struct {
    unsigned char data[32];
} wwrd_256;

typedef struct {
    unsigned short data[32];
} wwrd_512;

void bandwidth(wwrd_512 *output_port, wwrd_256 *input_port)
{
    #pragma HLS INTERFACE m_axi port=output_port offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=input_port offset=slave bundle=gmem1
    
    #pragma HLS INTERFACE s_axilite port=output_port bundle=control
    #pragma HLS INTERFACE s_axilite port=input_port bundle=control
    
    #pragma HLS INTERFACE s_axilite port=return bundle=control

    #pragma HLS DATA_PACK variable=input_port
    #pragma HLS DATA_PACK variable=output_port

    unsigned long blockindex;
    
    for (blockindex=0; blockindex < 10*1024*1024/32; blockindex++) {
        #pragma HLS pipeline
        int i;
        //int nonzero = 0;
        wwrd_512 reports;
        wwrd_256 inbuffer = input_port[blockindex];
        for (i = 0; i < 32 + inbuffer.data[0]; i++) {
            unsigned char indata = inbuffer.data[i];
            //if (indata != 0) nonzero++;
            unsigned short result = 0xFF00 + indata;
            reports.data[i] = result;
        }
//        if (nonzero != 0)
            output_port[blockindex] = reports;
    }
}
