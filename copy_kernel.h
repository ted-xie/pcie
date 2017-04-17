#ifndef COPY_KERNEL_H
#define COPY_KERNEL_H

#define DATA_SIZE 10*1024*1024
#define OUTPUT_SIZE 10*1024*1024

typedef struct {
    unsigned char  data[64];
} din_t;

typedef struct {
    unsigned char data[64];
} dout_t;

void bandwidth(dout_t *output_port, din_t *input_port);

#endif
