#Create solution
create_solution -name prj_hw_ku3_2ddr -force
add_device xilinx:adm-pcie-ku3:2ddr:3.0

set_param compiler.preserveHlsOutput 1
set_param compiler.preserveXoccTempProjectDir true

#Add host code
add_files kernel_global_bandwidth.c
set_property -name host_cflags -value "-g -DKU3_2DDR" -objects [current_solution]

#Create binary, kernel, compute unit
create_opencl_binary bin_bandwidth
set_property region OCL_REGION_0 [get_opencl_binary bin_bandwidth]

create_kernel -type c bandwidth
add_files -kernel [get_kernels bandwidth] Simple.c
create_compute_unit -opencl_binary [get_opencl_binary bin_bandwidth] -kernel [get_kernels bandwidth] -name bandwidth0

#create individual master ports for each global memory pointer and
#connect them to the desired memory bank
#set_property max_memory_ports true [get_kernels bandwidth]
#set_property memory_port_data_width 512 [get_kernels bandwidth]
#set_property memory_port_data_width 256 [get_kernels bandwidth]
#set_property memory_port_data_width 128 [get_kernels bandwidth]

map_connect  -opencl_binary [get_opencl_binary bin_bandwidth] \
    -src_type "kernel" -src_name "bandwidth0"   -src_port "M_AXI_GMEM0" \
    -dst_type "core"   -dst_name "OCL_REGION_0" -dst_port "M00_AXI"

map_connect  -opencl_binary [get_opencl_binary bin_bandwidth] \
    -src_type "kernel" -src_name "bandwidth0"   -src_port "M_AXI_GMEM1" \
    -dst_type "core"   -dst_name "OCL_REGION_0" -dst_port "M01_AXI"

#compile for emulation
compile_emulation -flow cpu -opencl_binary [get_opencl_binary bandwidth]
run_emulation -flow cpu 

#Create estimated resource usage and latency report
report_estimate

#compile for hw emulation
#compile_emulation -flow hardware -opencl_binary [get_opencl_binary bandwidth]
#run_emulation -flow hardware 

#Build and package system
build_system
package_system
