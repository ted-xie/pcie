#include <string.h>
#include <hls_stream.h>
#define BUFSIZE 64

typedef struct {
    unsigned char data[32];
} wwrd_256;

typedef struct {
    unsigned short data[32];
} wwrd_512;

void read(wwrd_256 * indata, hls::stream<wwrd_256> * buf) {
    #pragma HLS inline=off
    for (int i = 0; i < BUFSIZE; i++) { // read 64 words
        #pragma HLS PIPELINE
        buf->write(indata[i]);
    }
}

void write(wwrd_512 * outdata, hls::stream<wwrd_512> * buf) {
    #pragma HLS inline=off
    for (int i = 0; i < BUFSIZE; i++) { // write 64 words
        #pragma HLS PIPELINE
        outdata[i] = buf->read();
    }
    
}

void compute(hls::stream<wwrd_256> * indata, hls::stream<wwrd_512> * outdata) {
    #pragma HLS inline=off
    unsigned long block;

    for (int i = 0; i < BUFSIZE; i++) {
        wwrd_256 inbuffer = indata->read();
        wwrd_512 outbuffer;
        for (int j = 0; j < 32; j++) {
            unsigned short result = 0xAB00 + inbuffer.data[j];
            outbuffer.data[j] = result;
        }
        outdata->write(outbuffer);
    }
}

void read_compute_write(wwrd_256 * indata, wwrd_512 * outdata) {
#pragma HLS DATAFLOW
    hls::stream<wwrd_256> inbuffer("inbuffer");
    hls::stream<wwrd_512> outbuffer("outbuffer");
#pragma HLS stream variable=inbuffer depth=1
#pragma HLS stream variable=outbuffer depth=1

    read(indata, &inbuffer);
    compute(&inbuffer, &outbuffer);
    write(outdata, &outbuffer);
}

//extern "C"
void bandwidth(wwrd_512 * outbuffer, wwrd_256 * inbuffer) {
#pragma HLS INTERFACE m_axi port=inbuffer offset=slave bundle=gmem1 num_write_outstanding=16 num_read_outstanding=16 max_write_burst_length=16 max_read_burst_length=16
#pragma HLS INTERFACE m_axi port=outbuffer offset=slave bundle=gmem0 num_write_outstanding=16 num_read_outstanding=16 max_write_burst_length=16 max_read_burst_length=16

#pragma HLS INTERFACE s_axilite port=inbuffer bundle=control
#pragma HLS INTERFACE s_axilite port=outbuffer bundle=control

#pragma HLS INTERFACE s_axilite port=return bundle=control

#pragma HLS DATA_PACK variable=outbuffer
#pragma HLS DATA_PACK variable=inbuffer

    unsigned long block;
    
    for (block = 0; block < 10*1024*1024/32; block+=BUFSIZE) {
        read_compute_write(&inbuffer[block], &outbuffer[block]);
    }
    
}
