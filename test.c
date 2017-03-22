#include <stdio.h>
#include "copy_kernel.h"

int main() {
	unsigned long size_bytes = DATA_SIZE;
	unsigned char * inputs = (unsigned char *) malloc(size_bytes);
	unsigned short * outputs = (unsigned short *) malloc(sizeof(short) * size_bytes);

	// Populate inputs
	int i;
	for (i = 0; i < size_bytes; i++) {
		inputs[i] = i;
	}

	printf("Starting test...\n");
	bandwidth((wwrd_512 *) outputs, (wwrd_256 *) inputs);

	free(inputs);
	free(outputs);

	printf("Test completed!\n");
	return 0;
}
