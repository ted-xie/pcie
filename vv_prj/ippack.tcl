open_project ../vhls_prj/test_io/solution1/impl/verilog/project.xpr

ipx::package_project -root_dir ../ip_repo -vendor xilinx.com -library hls -taxonomy /UserIP -generated_files -import_files -set_current false
ipx::edit_ip_in_project -upgrade true -name tmp_edit_project -directory ../ip_repo ../ip_repo/component.xml
set_property top bandwidth [get_filesets sim_1]
ipx::infer_bus_interface ap_rst_n xilinx.com:signal:reset_rtl:1.0 [ipx::current_core]
set_property name Reg [ipx::get_address_blocks reg0 -of_objects [ipx::get_memory_maps s_axi_control -of_objects [ipx::current_core]]]
set_property core_revision 2 [ipx::current_core]
ipx::add_bus_parameter ASSOCIATED_BUSIF [ipx::get_bus_interfaces ap_clk -of_objects [ipx::current_core]]
set_property value m_axi_gmem0:m_axi_gmem1:s_axi_control [ipx::get_bus_parameters ASSOCIATED_BUSIF -of_objects [ipx::get_bus_interfaces ap_clk -of_objects [ipx::current_core]]]
ipx::add_bus_parameter ASSOCIATED_RESET [ipx::get_bus_interfaces ap_clk -of_objects [ipx::current_core]]
set_property value ap_rst_n [ipx::get_bus_parameters ASSOCIATED_RESET -of_objects [ipx::get_bus_interfaces ap_clk -of_objects [ipx::current_core]]]
ipx::create_xgui_files [ipx::current_core]
ipx::update_checksums [ipx::current_core]
ipx::save_core [ipx::current_core]

close_project
