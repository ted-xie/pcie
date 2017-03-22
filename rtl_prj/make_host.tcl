#Create solution
open_solution prj_hw_ku3_2ddr/prj_hw_ku3_2ddr.spr

set_param compiler.preserveHlsOutput 1
set_param compiler.preserveXoccTempProjectDir true

#Add host code
delete_files ../kernel_global_bandwidth.c ../copy_kernel.h
add_files ../kernel_global_bandwidth.c ../copy_kernel.h
set_property -name host_cflags -value "-g -DKU3_2DDR" -objects [current_solution]

