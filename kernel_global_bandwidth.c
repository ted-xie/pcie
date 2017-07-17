#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <CL/opencl.h>
#ifdef KU3_2DDR
#include <CL/cl_ext.h>
#endif

#include <unistd.h>
#include <sys/time.h>
#include "settings.h"
/////////////////////////////////////////////////////////////////////////////////
//load_file_to_memory
//Allocated memory for and load file from disk memory
//Return value
// 0   Success
//-1   Failure to open file
//-2   Failure to allocate memory
int load_file_to_memory(const char *filename, char **result,size_t *inputsize)
{
    int size = 0;
    FILE *f = fopen(filename, "rb");
    if (f == NULL) {
        *result = NULL;
        return -1; // -1 means file opening fail
    }
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    *result = (char *)malloc(size+1);
    if (size != fread(*result, sizeof(char), size, f))
        {
            free(*result);
            return -2; // -2 means file reading fail
        }
    fclose(f);
    (*result)[size] = 0;
    if(inputsize!=NULL) (*inputsize)=size;
    return 0;
}


/////////////////////////////////////////////////////////////////////////////////
//opencl_setup
//Create context for Xilinx platform, Accelerator device
//Create single command queue for accelerator device
//Create program object with clCreateProgramWithBinary using given xclbin file name
//Return value
// 0    Success
//-1    Error
//-2    Failed to load XCLBIN file from disk
//-3    Failed to clCreateProgramWithBinary
int opencl_setup(const char *xclbinfilename, cl_platform_id *platform_id,
                 cl_device_id *devices, cl_device_id *device_id, cl_context  *context,
                 cl_command_queue *command_queue, cl_program *program,
                 char *cl_platform_name, const char *target_device_name) {

    char cl_platform_vendor[1001];
    char cl_device_name[1001];
    cl_int err;
    cl_uint num_devices;
    unsigned int device_found = 0;

    // Get first platform
    err = clGetPlatformIDs(1,platform_id,NULL);
    if (err != CL_SUCCESS) {
        printf("ERROR: Failed to find an OpenCL platform!\n");
        printf("ERROR: Test failed\n");
        return -1;
    }
    err = clGetPlatformInfo(*platform_id,CL_PLATFORM_VENDOR,1000,(void *)cl_platform_vendor,NULL);
    if (err != CL_SUCCESS) {
        printf("ERROR: clGetPlatformInfo(CL_PLATFORM_VENDOR) failed!\n");
        printf("ERROR: Test failed\n");
        return -1;
    }
    printf("CL_PLATFORM_VENDOR %s\n",cl_platform_vendor);
    err = clGetPlatformInfo(*platform_id,CL_PLATFORM_NAME,1000,(void *)cl_platform_name,NULL);
    if (err != CL_SUCCESS) {
            printf("ERROR: clGetPlatformInfo(CL_PLATFORM_NAME) failed!\n");
            printf("ERROR: Test failed\n");
            return -1;
    }
    printf("CL_PLATFORM_NAME %s\n",cl_platform_name);

    // Get Accelerator compute device
    int accelerator = 1;
    err = clGetDeviceIDs(*platform_id, CL_DEVICE_TYPE_ACCELERATOR, 16, devices, &num_devices);
    if (err != CL_SUCCESS) {
        printf("ERROR: Failed to create a device group!\n");
        printf("ERROR: Test failed\n");
        return -1;
    }

    //iterate all devices to select the target device.
    for (int i=0; i<num_devices; i++) {
        err = clGetDeviceInfo(devices[i], CL_DEVICE_NAME, 1024, cl_device_name, 0);
        if (err != CL_SUCCESS) {
            printf("Error: Failed to get device name for device %d!\n", i);
            printf("Test failed\n");
            return EXIT_FAILURE;
        }
        //printf("CL_DEVICE_NAME %s\n", cl_device_name);
        if(strcmp(cl_device_name, target_device_name) == 0) {
            *device_id = devices[i];
            device_found = 1;
            printf("Selected %s as the target device\n", cl_device_name);
        }
    }

    if (!device_found) {
        printf("Target device %s not found. Exit.\n", target_device_name);
        return EXIT_FAILURE;
    }

    // Create a compute context containing accelerator device
    (*context)= clCreateContext(0, 1, device_id, NULL, NULL, &err);
    if (!(*context))
        {
            printf("ERROR: Failed to create a compute context!\n");
            printf("ERROR: Test failed\n");
            return -1;
        }

    // Create a command queue for accelerator device
    (*command_queue) = clCreateCommandQueue(*context, *device_id, CL_QUEUE_PROFILING_ENABLE, &err);
    if (!(*command_queue))
        {
            printf("ERROR: Failed to create a command commands!\n");
            printf("ERROR: code %i\n",err);
            printf("ERROR: Test failed\n");
            return -1;
        }

    // Load XCLBIN file binary from disk
    int status;
    unsigned char *kernelbinary;
    printf("loading %s\n", xclbinfilename);
    size_t xclbinlength;
    err = load_file_to_memory(xclbinfilename, (char **) &kernelbinary,&xclbinlength);
    if (err != 0) {
        printf("ERROR: failed to load kernel from xclbin: %s\n", xclbinfilename);
        printf("ERROR: Test failed\n");
        return -2;
    }

    // Create the program from XCLBIN file, configuring accelerator device
    (*program) = clCreateProgramWithBinary(*context, 1, device_id, &xclbinlength, (const unsigned char **) &kernelbinary, &status, &err);
    if ((!(*program)) || (err!=CL_SUCCESS)) {
        printf("ERROR: Failed to create compute program from binary %d!\n", err);
        printf("ERROR: Test failed\n");
        return -3;
    }

    // Build the program executable (no-op)
    err = clBuildProgram(*program, 0, NULL, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
            size_t len;
            char buffer[2048];
            printf("ERROR: Failed to build program executable!\n");
            clGetProgramBuildInfo(*program, *device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
            printf("%s\n", buffer);
            printf("ERROR: Test failed\n");
            return -1;
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////
//main

int main(int argc, char** argv)
{

    //change the line below to match the target device
#ifdef KU3_2DDR
    const char *target_device_name = "xilinx:adm-pcie-ku3:2ddr:3.0";
#elif KU3_1DDR
    const char *target_device_name = "xilinx:adm-pcie-ku3:1ddr:2.1";
#else
    const char *target_device_name = "xilinx:adm-pcie-7v3:1ddr:3.0";
#endif

    // binary container name is bin_bandwidth unless path is provided on command line
    const char *binary_container_name = argc > 1 ? argv[1] : "bin_bandwidth.xclbin";


    int err;
    unsigned int i;
    size_t globalbuffersize = DATA_SIZE;
    size_t globaloutputsize = OUTPUT_SIZE;
    printf("INFO: Using %d-bit size_t...\n", 8*sizeof(size_t));
    printf("INFO: Input data size is %lu bytes.\n", DATA_SIZE);
    printf("INFO: Output data size is %lu bytes.\n", OUTPUT_SIZE);

    //opencl setup
    cl_platform_id platform_id;
    cl_device_id device_id;
    cl_device_id devices[16];  // compute device id
    cl_context context;
    cl_command_queue command_queue;
    cl_program program;
    char cl_platform_name[1001];

    //variables for profiling
    uint64_t nsduration1, nsduration2;
    struct timeval start_tin, end_tin, start_tout, end_tout, start_t1, end_t1, start_t2, end_t2;
    float tin, tout, t1, t2;

    err = opencl_setup(binary_container_name, &platform_id, devices, &device_id,
                       &context, &command_queue, &program, cl_platform_name,
                       target_device_name);
    if(err==-1){
        printf("Error : general failure setting up opencl context\n");
        return -1;
    }
    if(err==-2) {
        printf("Error : failed to bandwidth.xclbin from disk\n");
        return -1;
    }
    if(err==-3) {
        printf("Error : failed to clCreateProgramWithBinary with contents of xclbin\n");
    }

    //access the ACCELERATOR kernel
    cl_int clstatus;
    cl_kernel kernel = clCreateKernel(program, "bandwidth", &clstatus);
    if (!kernel || clstatus != CL_SUCCESS) {
        printf("Error: Failed to create compute kernel!\n");
        printf("Error: Test failed\n");
        return -1;
    }

    //input buffer
    unsigned char *input_host = ((unsigned char *)malloc(globalbuffersize));
    if(input_host==NULL) {
        printf("Error: Failed to allocate host side copy of OpenCL source buffer of size %i\n",globalbuffersize);
        return -1;
    }
	//output buffer
    unsigned char *output_host = ((unsigned char *)malloc(globaloutputsize));
    if(output_host==NULL) {
        printf("Error: Failed to allocate host side copy of OpenCL source buffer of size %i\n",globaloutputsize);
        return -1;
    }

    if (argc < 3) {
        printf("WARNING: Usage: ./<executable> <xclbin> <input_file> <print output, y/n>\n");
        printf("INFO: Automatically sending dummy data...\n");
        for(i=0; i<globalbuffersize; i++)
            input_host[i]=i;
    } else { 
        FILE *fp = fopen(argv[2], "rb");
        fread(input_host, globalbuffersize, 1, fp);
        fclose(fp);
    }

    unsigned int printOutput = 0;
    if (argc >= 4) {
        if (argv[3][0] == 'y')
            printOutput = 1;
    } else {
        printf("INFO: Not going to print out outputs...\n");
    }

    cl_mem input_buffer;
#ifdef KU3_2DDR
    cl_mem_ext_ptr_t input_buffer_ext;
    input_buffer_ext.flags = XCL_MEM_DDR_BANK0;
    input_buffer_ext.obj = NULL;
    input_buffer_ext.param = 0;

    input_buffer = clCreateBuffer(context,
                                  CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX,
                                  globalbuffersize,
                                  &input_buffer_ext,
                                  &err);
#else
    input_buffer = clCreateBuffer(context,
                                  CL_MEM_READ_WRITE,
                                  globalbuffersize,
                                  NULL,
                                  &err);
#endif
    if(err != CL_SUCCESS) {
        printf("Error: Failed to allocate OpenCL source buffer of size %i\n", globalbuffersize);
        return -1;
    }

    //output buffer
    cl_mem output_buffer;
#ifdef KU3_2DDR
    cl_mem_ext_ptr_t output_buffer_ext;
    output_buffer_ext.flags = XCL_MEM_DDR_BANK1;
    output_buffer_ext.obj = NULL;
    output_buffer_ext.param = 0;

    output_buffer = clCreateBuffer(context,
                                   CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX,
                                   globaloutputsize,
                                   &output_buffer_ext,
                                   &err);
#else
    output_buffer = clCreateBuffer(context,
                                   CL_MEM_READ_WRITE,
                                   globaloutputsize,
                                   NULL,
                                   &err);
#endif
    if (err != CL_SUCCESS) {
        printf("Error: Failed to allocate worst case OpenCL output buffer of size %i\n",globaloutputsize);
        return -1;
    }

    double dbytes = globalbuffersize;
    double dmbytes = dbytes / (((double)1024) * ((double)1024));
    printf("Starting kernel to read/write %.0lf MB bytes from/to global memory... \n", dmbytes);

    gettimeofday(&start_tin, NULL);

    //Write input buffer
    //Map input buffer for PCIe write
    unsigned char *map_input_buffer;
    map_input_buffer = (unsigned char *) clEnqueueMapBuffer(command_queue,
                                                            input_buffer,
                                                            CL_FALSE,
                                                            CL_MAP_WRITE_INVALIDATE_REGION,
                                                            0,
                                                            globalbuffersize,
                                                            0,
                                                            NULL,
                                                            NULL,
                                                            &err);
    if (err != CL_SUCCESS) {
        printf("Error: Failed to clEnqueueMapBuffer OpenCL buffer\n");
        printf("Error: Test failed\n");
        return -1;
    } else {
        printf("SUCCESS: Allocated input buffer memory.\n");
    }
    clFinish(command_queue);

    //prepare data to be written to the device
    //for(i=0; i<globalbuffersize; i++)
    //    map_input_buffer[i] = input_host[i];
    memcpy(map_input_buffer, input_host, globalbuffersize);

    cl_event event1;
    err = clEnqueueUnmapMemObject(command_queue,
                                  input_buffer,
                                  map_input_buffer,
                                  0,
                                  NULL,
                                  &event1);
    if (err != CL_SUCCESS) {
        printf("Error: Failed to copy input dataset to OpenCL buffer\n");
        printf("Error: Test failed\n");
        return -1;
    } else {
        printf("SUCCESS: Copied input data to input buffer.\n");
    }
    clFinish(command_queue);

	/*err  = clEnqueueWriteBuffer(command_queue, input_buffer, CL_TRUE, 0, globalbuffersize, input_host, 0, NULL, NULL);
    if (err != CL_SUCCESS) printf("Error in clEnqueueWriteBuffer input_buffer\n");*///Slower

    gettimeofday(&end_tin, NULL);

    int cycles = 32; // never change this!!!
    //execute kernel
    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &output_buffer);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &input_buffer);

    if (err != CL_SUCCESS) {
        printf("ERROR: Failed to set kernel arguments! %d\n", err);
        printf("ERROR: Test failed\n");
        return EXIT_FAILURE;
    } else {
        printf("SUCCESS: Set kernel arguments.\n");
    }

    size_t global[1];
    size_t local[1];
    global[0]=1;
    local[0]=1;

    cl_event ndrangeevent1;
    cl_event ndrangeevent2;

    gettimeofday(&start_t1, NULL);

    err = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, global, local, 0, NULL, &ndrangeevent1);
    if (err != CL_SUCCESS) {
        printf("ERROR: Failed to execute kernel %d\n", err);
        printf("ERROR: Test failed\n");
        return EXIT_FAILURE;
    } 
    printf("SUCCESS: Started executing kernel.\n");
    
    clWaitForEvents(1, &ndrangeevent1);
    printf("SUCCESS: Kernel finished executing.\n");

    gettimeofday(&end_t1, NULL);

    gettimeofday(&start_tout, NULL);

    //copy results back from OpenCL buffer
    unsigned char *map_output_buffer;
    map_output_buffer = (unsigned char *)clEnqueueMapBuffer(command_queue,
                                                            output_buffer,
                                                            CL_FALSE,
                                                            CL_MAP_READ,
                                                            0,
                                                            globaloutputsize,
                                                            0,
                                                            NULL,
                                                            &event1,
                                                            &err);
    if (err != CL_SUCCESS) {
        printf("ERROR: Failed to read output size buffer %d\n", err);
        printf("ERROR: Test failed\n");
        return EXIT_FAILURE;
    } else {
        printf("SUCCESS: Read output buffer.\n");
    }
    clFinish(command_queue);

    //copy from device to host
    //for(i=0; i<globalbuffersize; i++)
    //    output_host[i] = map_output_buffer[i];
    memcpy(output_host, map_output_buffer, globaloutputsize);

    gettimeofday(&end_tout, NULL);

    //profiling information
    uint64_t nstimestart, nstimeend;

    clGetEventProfilingInfo(ndrangeevent1, CL_PROFILING_COMMAND_START, sizeof(uint64_t), ((void *)(&nstimestart)), NULL);
    clGetEventProfilingInfo(ndrangeevent1, CL_PROFILING_COMMAND_END,   sizeof(uint64_t), ((void *)(&nstimeend)),   NULL);
    nsduration1 = nstimeend-nstimestart;

    double dnsduration1 = ((double)nsduration1);
    double dsduration1 = dnsduration1 / ((double) 1000000000);

    double bpersec1 = (dbytes/dsduration1);
    double mbpersec1 = bpersec1 / ((double) 1024*1024 );

    t1 = (end_t1.tv_sec + end_t1.tv_usec/(double)(1000000)) - (start_t1.tv_sec + start_t1.tv_usec/(double)(1000000));

    double bpersec1_ = (dbytes/t1);
    double mbpersec1_ = bpersec1_ / ((double) 1024*1024 );


    //add clean up code
    //

/*    int numErrors = 0;
    for (i = 0; i < globalbuffersize; i++) {
        if ((i & 1) == 0) {
            if (output_host[i] != (input_host[i] + 0xFA)) {numErrors++;}
        } else {
            if (output_host[i] != (input_host[i] + 0xFB)) {numErrors++;}
        }
    }
    printf("numErrors: %d\n", numErrors); 
*/

    for (i = 0; i < globalbuffersize; i++) {
        if (printOutput && output_host[i] != 0)
            printf("Cycle %d report\n", i);
    }
    printf("Kernel completed read/write %.0lf MB bytes from/to global memory.\n", dmbytes);
    printf("clGetEventProfilingInfo - Execution time (1st) = %f (sec)\n", dsduration1);
    printf("clGetEventProfilingInfo - Concurrent Read and Write Throughput (1st) = %f (MB/sec)\n", mbpersec1);
    printf("\n");
    printf("gettimeofday - Execution time (1st) = %f (sec)\n", t1);
    printf("gettimeofday - Concurrent Read and Write Throughput (1st) = %f (MB/sec);\n", mbpersec1_);
    printf("\n");
    printf("Overhead of kernel call - 1st call = %f(sec)\n", t1 - dsduration1);

    tin = (end_tin.tv_sec  + end_tin.tv_usec/(double)(1000000))  - (start_tin.tv_sec  + start_tin.tv_usec/(double)(1000000));
    tout= (end_tout.tv_sec + end_tout.tv_usec/(double)(1000000)) - (start_tout.tv_sec + start_tout.tv_usec/(double)(1000000));

    double bpersec_in = (dbytes/tin);
    double mbpersec_in = bpersec_in / ((double) 1024*1024 );
    double bpersec_out = (dbytes/tout);
    double mbpersec_out = bpersec_out / ((double) 1024*1024 );

    printf("\n");
    printf("gettimeofday - Transfer_in time = %f (sec); Transfer_out time = %f (sec) \n", tin, tout);
    printf("gettimeofday - Transfer_in Throughput = %f (MB/sec); Transfer_out Throughput = %f (MB/sec) \n", mbpersec_in, mbpersec_out);
    return EXIT_SUCCESS;

}
