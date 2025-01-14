set(RTS_PLUGIN some_plugin)

include(WiresharkPlugin)

# Plugin name and version info (major minor micro extra)
set_module_info(${RTS_PLUGIN} 0 0 1 0)

set(DISSECTOR_SRC
	${RTS_PLUGIN}.cpp
)

set(PLUGIN_FILES
	plugin.c
	${DISSECTOR_SRC}
)

set_source_files_properties(
	${PLUGIN_FILES}
	PROPERTIES
	COMPILE_FLAGS "${WERROR_COMMON_FLAGS}"
)

register_plugin_files(plugin.c
	plugin
	${DISSECTOR_SRC}
)

add_wireshark_plugin_library(${RTS_PLUGIN} epan)

target_link_libraries(${RTS_PLUGIN} tll epan)
target_compile_features(${RTS_PLUGIN} PRIVATE cxx_std_17)

install_plugin(${RTS_PLUGIN} epan)

file(GLOB DISSECTOR_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
CHECKAPI(
	NAME
	  ${RTS_PLUGIN}
	SWITCHES
	  --${RTS_PLUGIN} dissectors-prohibited
	  --${RTS_PLUGIN} dissectors-restricted
	SOURCES
	  ${DISSECTOR_SRC}
	  ${DISSECTOR_HEADERS}
)

#
# Editor modelines  -  https://www.wireshark.org/tools/modelines.html
#
# Local variables:
# c-basic-offset: 8
# tab-width: 8
# indent-tabs-mode: t
# End:
#
# vi: set shiftwidth=8 tabstop=8 noexpandtab:
# :indentSize=8:tabSize=8:noTabs=false:
#
