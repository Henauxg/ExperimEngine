cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
message("CMake version is : ${CMAKE_VERSION}")

project(ExperimEngine VERSION 0.0.1)

 set(CMAKE_CXX_STANDARD 20)
 set(CMAKE_CXX_STANDARD_REQUIRED True)

# Executable and header configuration
configure_file(ExperimEngineConfig.h.in ExperimEngineConfig.h)
add_executable(ExperimEngine "")

# Sources & headers inclusion
add_subdirectory(src/tests)
add_subdirectory(src/engine)
add_subdirectory(src/engine/log)
add_subdirectory(src/engine/render)
add_subdirectory(src/engine/render/imgui/)
add_subdirectory(src/engine/render/imgui/lib)
add_subdirectory(src/engine/render/imgui/widgets)
add_subdirectory(src/engine/utils)
if(NOT WEB_TARGET)
	add_subdirectory(src/engine/render/vlk)
	add_subdirectory(src/engine/render/vlk/resources)
	add_subdirectory(src/engine/render/imgui/vlk)
	# Lua interpreter/jit not yet compiled to WASM
	add_subdirectory(src/tests/lua)
else()
	add_subdirectory(src/engine/render/wgpu)
	add_subdirectory(src/engine/render/wgpu/resources)
	add_subdirectory(src/engine/render/imgui/wgpu)
endif()

# Here if needed : add external CMake build files :
# add_subdirectory(external/shaderc)

# Common includes
target_include_directories(ExperimEngine PUBLIC
	# For CMake generated headers
    ${PROJECT_BINARY_DIR}
	${PROJECT_SOURCE_DIR}/includes
	${PROJECT_SOURCE_DIR}/includes/lua
	${PROJECT_SOURCE_DIR}/src
)
# Web-target includes/compile definitions
if(WEB_TARGET)
	# Unused as of now since a custom template is used
	# set(CMAKE_EXECUTABLE_SUFFIX ".html")
	target_include_directories(ExperimEngine PUBLIC
		# Emscripten headers
		"${EMSCRIPTEN_ROOT_PATH}/system/include"
	)
	# This is already defined by the toolchain,
	# but redefined here so that Visual Studio/Intellisense can pick it up
	add_compile_definitions(__EMSCRIPTEN__)
endif()

# Windows specific : linking
if(WIN32)
	if(CMAKE_BUILD_TYPE STREQUAL Release)
		message("Console disabled in win32-release build")
		set_target_properties(
			ExperimEngine PROPERTIES
			LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE"
			LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
	endif()
endif()

#  Web-target specific : linking and post-build commands
if(WEB_TARGET)
	# Preload runtime files to a virtual file system
	set_target_properties(ExperimEngine PROPERTIES LINK_FLAGS "-s USE_SDL=2 -s USE_WEBGPU=1 --preload-file ${PROJECT_SOURCE_DIR}/data@/data") 

	set(TOOLS_DIR ${PROJECT_SOURCE_DIR}/tools)
	add_custom_command(TARGET ExperimEngine POST_BUILD
		# Web html template
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${TOOLS_DIR}/emscripten/index.html ${CMAKE_BINARY_DIR}
	)

# Native specific : linking and post-build commands
else()
	# Link with libraries.
	# TODO : x64/x86 flags
	set(LIBS_DIR ${PROJECT_SOURCE_DIR}/libs)
	target_link_libraries(ExperimEngine PRIVATE 
		${LIBS_DIR}/lua/lua51.lib
		${LIBS_DIR}/SDL2/x64/SDL2.lib
		${LIBS_DIR}/SDL2/x64/SDL2main.lib
		# shaderc
	)

	# Commands after each non-web build
	add_custom_command(TARGET ExperimEngine POST_BUILD
		# Binaries
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${LIBS_DIR}/lua/lua51.dll ${CMAKE_BINARY_DIR}
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${LIBS_DIR}/SDL2/x64/SDL2.dll ${CMAKE_BINARY_DIR}
	)
endif()

# Common commands after build
add_custom_command(TARGET ExperimEngine POST_BUILD
	# Data files
	COMMAND ${CMAKE_COMMAND} -E copy_directory  ${PROJECT_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/data
)
