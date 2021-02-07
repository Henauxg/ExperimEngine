cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
message("CMake version is : ${CMAKE_VERSION}")

project(ExperimEngine VERSION 0.0.1)

 set(CMAKE_CXX_STANDARD 20)
 set(CMAKE_CXX_STANDARD_REQUIRED True)

# Header configuration
configure_file(ExperimEngineConfig.h.in ExperimEngineConfig.h)

# Engine library target
set(ENGINE_LIB_TARGET_NAME LibExperimEngine)
add_library(${ENGINE_LIB_TARGET_NAME} STATIC "")

#####################
# Engine sources
#####################

add_subdirectory(src/engine)
add_subdirectory(src/engine/log)
add_subdirectory(src/engine/render)
add_subdirectory(src/engine/render/imgui/)
add_subdirectory(src/engine/render/imgui/lib)
add_subdirectory(src/engine/render/imgui/widgets)
add_subdirectory(src/engine/render/resources)
add_subdirectory(src/engine/utils)
if(WEB_TARGET)
	add_subdirectory(src/engine/render/wgpu)
	add_subdirectory(src/engine/render/wgpu/resources)
	add_subdirectory(src/engine/render/imgui/wgpu)
else()
	add_subdirectory(src/engine/render/vlk)
	add_subdirectory(src/engine/render/vlk/resources)
	add_subdirectory(src/engine/render/imgui/vlk)
endif()

# Here if needed : add external CMake build files :
# add_subdirectory(external/shaderc)

#####################
# Engine includes
#####################

# Common
target_include_directories(${ENGINE_LIB_TARGET_NAME} PUBLIC
    ${PROJECT_BINARY_DIR} # For CMake generated headers
	${PROJECT_SOURCE_DIR}/includes
	${PROJECT_SOURCE_DIR}/includes/lua
	${PROJECT_SOURCE_DIR}/src
)

# Web-target
if(WEB_TARGET)
	# Unused line below : can be used to generate the emscripten html wrapper, since a custom template is used
	# set(CMAKE_EXECUTABLE_SUFFIX ".html")
	target_include_directories(${ENGINE_LIB_TARGET_NAME} PUBLIC
		"${EMSCRIPTEN_ROOT_PATH}/system/include" # Emscripten headers
	)

# Native
else()
	target_include_directories(${ENGINE_LIB_TARGET_NAME} PUBLIC
		${PROJECT_SOURCE_DIR}/includes/dawn
	)
endif()

#####################
# Linking libraries to the engine
#####################

# Native specific
if(NOT WEB_TARGET)
	# Link with libraries.
	# TODO : x64/x86 flags
	set(LIBS_DIR ${PROJECT_SOURCE_DIR}/libs)
	target_link_libraries(${ENGINE_LIB_TARGET_NAME} PRIVATE
		${LIBS_DIR}/lua/lua51.lib
		${LIBS_DIR}/SDL2/x64/SDL2.lib
		${LIBS_DIR}/SDL2/x64/SDL2main.lib
		#${LIBS_DIR}/dawn/dawn_native.dll.lib
		#${LIBS_DIR}/dawn/dawn_platform.dll.lib
		# shaderc
	)
endif()

#####################
# Compile definitions
#####################

if(WEB_TARGET)
# This is already defined by the toolchain, but redefined here so that Visual Studio/Intellisense can pick it up
target_compile_definitions(${ENGINE_LIB_TARGET_NAME} PUBLIC __EMSCRIPTEN__)

endif()

target_compile_definitions(${ENGINE_LIB_TARGET_NAME} PUBLIC __EXPERIMENGINE__)

#####################
# For engine applications
#####################

# Function that can be used by an Engine application to :
#   - set the includes directories to access the engine API
#   - set the appropriate link flags according to the target environment (Web, Win, Release, ...)
#   - set the link dependency to the engine library
# argument 'engineTarget' should be the cmake target of the application
# argument 'engineRootPath' should be the path to the engine directory
function(add_experimengine_target engineTarget engineRootPath)
	message("Added ExperimEngine target ${engineTarget}") 

	# Include directories to access the engine API
	target_include_directories(${engineTarget} PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/includes
		${CMAKE_CURRENT_SOURCE_DIR}/includes/lua
		${CMAKE_CURRENT_SOURCE_DIR}/src
	)

	# Web-target properties
	if(WEB_TARGET)
		# Preload runtime files to a virtual file system
		# TODO Expose files as an argument
		set_target_properties(${engineTarget} PROPERTIES LINK_FLAGS "-s USE_SDL=2 -s USE_WEBGPU=1 -s ALLOW_MEMORY_GROWTH=1 --preload-file ${PROJECT_SOURCE_DIR}/data@/data")

	# Native properties
	else()
		# Windows specific : linking
		if(WIN32)
			if(CMAKE_BUILD_TYPE STREQUAL Release)
				message("Console disabled in win32-release build")
				set_target_properties(
					${engineTarget} PROPERTIES
					LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE"
					LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
			endif()
		endif()
	endif()

	# Post build commands

	# Web-target commands
	if(WEB_TARGET)
		add_custom_command(TARGET ${engineTarget} POST_BUILD
			# Web html template
			COMMAND ${CMAKE_COMMAND} -E copy_if_different ${engineRootPath}/tools/emscripten/index.html $<TARGET_FILE_DIR:${engineTarget}>
		)

	# Native commands
	else()
		add_custom_command(TARGET ${engineTarget} POST_BUILD
			# Binaries
			COMMAND ${CMAKE_COMMAND} -E copy_if_different ${engineRootPath}/libs/lua/lua51.dll $<TARGET_FILE_DIR:${engineTarget}>
			COMMAND ${CMAKE_COMMAND} -E copy_if_different ${engineRootPath}/libs/SDL2/x64/SDL2.dll $<TARGET_FILE_DIR:${engineTarget}>
		)
	endif()

	# Common commands
	add_custom_command(TARGET ${engineTarget} POST_BUILD
		# Data files
		COMMAND ${CMAKE_COMMAND} -E copy_directory  ${engineRootPath}/data $<TARGET_FILE_DIR:${engineTarget}>/data
	)

	target_link_libraries(${engineTarget} LibExperimEngine)
endfunction()


#####################
# Build a test application
#####################

# If this is the main project, not if included through add_subdirectory
#if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	message("Building sample application")

	set(ENGINE_APP_BUILD_TARGET_NAME ExperimEngineTest)
	add_executable(${ENGINE_APP_BUILD_TARGET_NAME} "")

	add_subdirectory(src/tests)
	if(NOT WEB_TARGET)
		# Lua interpreter/jit not yet compiled to WASM
		add_subdirectory(src/tests/lua)
	endif()

	add_experimengine_target(${ENGINE_APP_BUILD_TARGET_NAME} ${PROJECT_SOURCE_DIR})
#endif()
