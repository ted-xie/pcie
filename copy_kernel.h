#ifndef COPY_KERNEL_H
#define COPY_KERNEL_H

#include "settings.h"
#include "ap_cint.h"

typedef struct {
    uint512 data[MULT];
} dout_t;
void bandwidth(dout_t *output_port, unsigned char *input_port);

#endif
