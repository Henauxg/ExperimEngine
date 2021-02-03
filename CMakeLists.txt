cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
message("CMake version is : ${CMAKE_VERSION}")

project(ExperimEngine VERSION 0.0.1)

 set(CMAKE_CXX_STANDARD 20)
 set(CMAKE_CXX_STANDARD_REQUIRED True)

# Header configuration
configure_file(ExperimEngineConfig.h.in ExperimEngineConfig.h)
# Build the engine library
add_library(LibExperimEngine STATIC "")
set(ENGINE_BUILD_TARGET_NAME LibExperimEngine)

#####################
# Engine sources
#####################

# Sources & headers inclusion
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
else()
	add_subdirectory(src/engine/render/wgpu)
	add_subdirectory(src/engine/render/wgpu/resources)
	add_subdirectory(src/engine/render/imgui/wgpu)
endif()

# Here if needed : add external CMake build files :
# add_subdirectory(external/shaderc)

#####################
# Includes
#####################

# Common includes
target_include_directories(${ENGINE_BUILD_TARGET_NAME} PUBLIC
	# For CMake generated headers
    ${PROJECT_BINARY_DIR}
	${PROJECT_SOURCE_DIR}/includes
	${PROJECT_SOURCE_DIR}/includes/lua
	${PROJECT_SOURCE_DIR}/src
)

# Web-target
if(WEB_TARGET)
	# Unused line below can be used to generate the emscripten html wrapper, since a custom template is used
	# set(CMAKE_EXECUTABLE_SUFFIX ".html")
	target_include_directories(${ENGINE_BUILD_TARGET_NAME} PUBLIC
		# Emscripten headers
		"${EMSCRIPTEN_ROOT_PATH}/system/include"
	)
	# This is already defined by the toolchain,
	# but redefined here so that Visual Studio/Intellisense can pick it up
	add_compile_definitions(__EMSCRIPTEN__)

# Native
else()
	target_include_directories(${ENGINE_BUILD_TARGET_NAME} PUBLIC
		${PROJECT_SOURCE_DIR}/includes/dawn
	)
endif()

#####################
# Linking
#####################

#  Web-target specific
if(WEB_TARGET)

# Native specific
else()
	# Link with libraries.
	# TODO : x64/x86 flags
	set(LIBS_DIR ${PROJECT_SOURCE_DIR}/libs)
	target_link_libraries(${ENGINE_BUILD_TARGET_NAME} PRIVATE
		${LIBS_DIR}/lua/lua51.lib
		${LIBS_DIR}/SDL2/x64/SDL2.lib
		${LIBS_DIR}/SDL2/x64/SDL2main.lib
		#${LIBS_DIR}/dawn/dawn_native.dll.lib
		#${LIBS_DIR}/dawn/dawn_platform.dll.lib
		# shaderc
	)
endif()

#####################
# Build an application
#####################

if(NOT DEFINED ENGINE_APP_BUILD_TARGET_NAME)
	set(ENGINE_APP_BUILD_TARGET_NAME ExperimEngineTest)
	add_executable(${ENGINE_APP_BUILD_TARGET_NAME} "")

	add_subdirectory(src/tests)
	if(NOT WEB_TARGET)
		# Lua interpreter/jit not yet compiled to WASM
		add_subdirectory(src/tests/lua)
	endif()
	
	target_link_libraries(${ENGINE_APP_BUILD_TARGET_NAME} LibExperimEngine)
endif()

#####################
# Linking and post build commands
#####################

#  Web-target specific
if(WEB_TARGET)
	# Preload runtime files to a virtual file system
	set_target_properties(${ENGINE_APP_BUILD_TARGET_NAME} PROPERTIES LINK_FLAGS "-s USE_SDL=2 -s USE_WEBGPU=1 -s ALLOW_MEMORY_GROWTH=1 --preload-file ${PROJECT_SOURCE_DIR}/data@/data") 

	set(TOOLS_DIR ${PROJECT_SOURCE_DIR}/tools)
	add_custom_command(TARGET ${ENGINE_APP_BUILD_TARGET_NAME} POST_BUILD
		# Web html template
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${TOOLS_DIR}/emscripten/index.html ${CMAKE_BINARY_DIR}
	)

# Native specific
else()
	# Windows specific : linking
	if(WIN32)
		if(CMAKE_BUILD_TYPE STREQUAL Release)
			message("Console disabled in win32-release build")
			set_target_properties(
				${ENGINE_APP_BUILD_TARGET_NAME} PROPERTIES
				LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE"
				LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
		endif()
	endif()

	# Commands after each non-web build
	add_custom_command(TARGET ${ENGINE_BUILD_TARGET_NAME} POST_BUILD
		# Binaries
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${LIBS_DIR}/lua/lua51.dll ${CMAKE_BINARY_DIR}
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${LIBS_DIR}/SDL2/x64/SDL2.dll ${CMAKE_BINARY_DIR}
	)
endif()

# Common commands after build
add_custom_command(TARGET ${ENGINE_BUILD_TARGET_NAME} POST_BUILD
	# Data files
	COMMAND ${CMAKE_COMMAND} -E copy_directory  ${PROJECT_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/data
)
