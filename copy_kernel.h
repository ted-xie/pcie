#ifndef COPY_KERNEL_H
#define COPY_KERNEL_H
#define DATA_SIZE 10*1024*1024
#define OUTPUT_SIZE 10*1024*1024
typedef struct {
    unsigned short  data[32];
} din_t;

typedef struct {
    unsigned short data[32];
} dout_t;

void bandwidth(din_t *output_port, din_t *input_port);

#endif
