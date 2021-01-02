cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
message("CMake version is : ${CMAKE_VERSION}")

project(ExperimEngine VERSION 0.0.1)

 set(CMAKE_CXX_STANDARD 20)
 set(CMAKE_CXX_STANDARD_REQUIRED True)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
	add_compile_definitions(DEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL Release)
	# Disable console in Windows release
	IF(WIN32)
		SET(GUI_TYPE WIN32)
	ENDIF(WIN32)
endif()

configure_file(ExperimEngineConfig.h.in ExperimEngineConfig.h)
add_executable(ExperimEngine "")
add_subdirectory(src)
add_subdirectory(src/engine)
add_subdirectory(src/engine/log)
if(NOT WEB_TARGET)
	add_subdirectory(src/engine/render)
	add_subdirectory(src/engine/render/imgui/impl)
	add_subdirectory(src/engine/render/imgui/lib)
	add_subdirectory(src/engine/render/imgui/widgets)
	add_subdirectory(src/engine/render/vlk)
	add_subdirectory(src/engine/render/vlk/resources)
	add_subdirectory(src/engine/utils)
	add_subdirectory(src/test)
endif()

# Add external CMake build files :
# add_subdirectory(external/shaderc)

target_include_directories(ExperimEngine PUBLIC
	# For CMake generated headers
    ${PROJECT_BINARY_DIR}
	${PROJECT_SOURCE_DIR}/includes
	${PROJECT_SOURCE_DIR}/includes/lua
	${PROJECT_SOURCE_DIR}/src
)

if(WEB_TARGET)
	SET(CMAKE_EXECUTABLE_SUFFIX ".html")
	set_target_properties(
	   ExperimEngine
		PROPERTIES
		LINK_FLAGS
		"-O2 -s USE_SDL=2"
	) 
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

# Commands after each build
add_custom_command(TARGET ExperimEngine POST_BUILD
	# Lua Scripts
	COMMAND ${CMAKE_COMMAND} -E copy_directory  ${PROJECT_SOURCE_DIR}/scripts ${CMAKE_BINARY_DIR}/scripts
	# Data files
	COMMAND ${CMAKE_COMMAND} -E copy_directory  ${PROJECT_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/data
)
