#ifndef COPY_KERNEL_H
#define COPY_KERNEL_H
#define DATA_SIZE 10*1024*1024

typedef struct {
    unsigned char data[16];
} wwrd_256;

typedef struct {
    unsigned short data[16];
} wwrd_512;

void bandwidth(wwrd_512 *output_port, wwrd_256 *input_port);

#endif
