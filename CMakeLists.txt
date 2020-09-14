cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
message("CMake version is : ${CMAKE_VERSION}")

project(ExperimEngine VERSION 0.0.1)

 set(CMAKE_CXX_STANDARD 20)
 set(CMAKE_CXX_STANDARD_REQUIRED True)

# Disable console in release
if(CMAKE_BUILD_TYPE STREQUAL Debug)
	add_compile_definitions(DEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL Release)
	IF(WIN32)
		SET(GUI_TYPE WIN32)
	ENDIF(WIN32)
endif()

configure_file(ExperimEngineConfig.h.in ExperimEngineConfig.h)

add_executable(ExperimEngine "")
add_subdirectory(src)
add_subdirectory(src/engine)
add_subdirectory(src/engine/log)
add_subdirectory(src/engine/render)
add_subdirectory(src/engine/render/imgui/impl)
add_subdirectory(src/engine/render/imgui/lib)
add_subdirectory(src/engine/render/imgui/widgets)
add_subdirectory(src/engine/render/vlk)
add_subdirectory(src/engine/render/vlk/resources)
add_subdirectory(src/test)

set(LIBS_DIR ${PROJECT_SOURCE_DIR}/libs)

# Find installed Vulkan lib. Available in CMake 3.7 and later
find_package(Vulkan REQUIRED)

# Libraries.
# TODO : x64/x86 flags
target_link_libraries(ExperimEngine PUBLIC 
	${LIBS_DIR}/lua/lua51.lib
	${LIBS_DIR}/SDL2/x64/SDL2.lib
	${LIBS_DIR}/SDL2/x64/SDL2main.lib
	${Vulkan_LIBRARIES}
)

# Includes : ${PROJECT_BINARY_DIR} for CMake generated headers
target_include_directories(ExperimEngine PUBLIC
    ${PROJECT_BINARY_DIR}
	${PROJECT_SOURCE_DIR}/includes
	${PROJECT_SOURCE_DIR}/includes/lua
	${PROJECT_SOURCE_DIR}/src
)

# Commands after each build
add_custom_command(TARGET ExperimEngine POST_BUILD
	# Binaries
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${LIBS_DIR}/lua/lua51.dll ${CMAKE_BINARY_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${LIBS_DIR}/SDL2/x64/SDL2.dll ${CMAKE_BINARY_DIR}
	# Lua Scripts
	COMMAND ${CMAKE_COMMAND} -E copy_directory  ${PROJECT_SOURCE_DIR}/scripts ${CMAKE_BINARY_DIR}/scripts
	# Data files
	COMMAND ${CMAKE_COMMAND} -E copy_directory  ${PROJECT_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/data
)
