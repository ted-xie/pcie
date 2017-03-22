#define SIZE 16

typedef struct {
	unsigned short data[SIZE];
} din_t;

typedef struct {
	unsigned int data[SIZE];
} dout_t;

void bandwidth(dout_t *output_port, din_t *input_port)
{
    #pragma HLS INTERFACE m_axi port=output_port offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=input_port offset=slave bundle=gmem1

    #pragma HLS INTERFACE s_axilite port=output_port bundle=control
    #pragma HLS INTERFACE s_axilite port=input_port bundle=control

    #pragma HLS INTERFACE s_axilite port=return bundle=control

    #pragma HLS DATA_PACK variable=input_port
    #pragma HLS DATA_PACK variable=output_port

	unsigned long block;
	unsigned int i;

	for (block = 0; block < 10*1024*1024/SIZE; block++) {
		din_t din = input_port[block];
		dout_t dout;

#pragma HLS DATA_PACK variable=din
#pragma HLS DATA_PACK variable=dout

#pragma HLS RESOURCE variable=din core=RAM_2P_BRAM
#pragma HLS RESOURCE variable=dout core=RAM_2P_BRAM


		for (i = 0; i < SIZE; i++) {
#pragma HLS pipeline ii=1
			dout.data[i] =  din.data[i];
		}

		output_port[block] = dout;
	}
}
