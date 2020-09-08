cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

project(ExperimEngine VERSION 0.0.1)

 set(CMAKE_CXX_STANDARD 17)
 set(CMAKE_CXX_STANDARD_REQUIRED True)

configure_file(ExperimEngineConfig.h.in ExperimEngineConfig.h)

add_executable(ExperimEngine "")
add_subdirectory(src)
add_subdirectory(src/render)
add_subdirectory(src/render/imgui)
add_subdirectory(src/render/imgui/lib)
add_subdirectory(src/render/vlk)
add_subdirectory(src/test)

set(LIBS_DIR ${PROJECT_SOURCE_DIR}/libs)

# Find installed Vulkan lib. Available in CMake 3.7 and later
find_package(Vulkan REQUIRED)
# Else use the packaged one
if (NOT Vulkan_FOUND)
	find_library(Vulkan_LIBRARIES NAMES vulkan vulkan-1 PATHS ${LIBS_DIR}/vulkan)
	if (Vulkan_LIBRARIES)
		set(Vulkan_FOUND ON)
		message("Using packaged Vulkan library")
	else()
		message(FATAL_ERROR "Could not find Vulkan library")
	endif()
endif()

# Libraries.
# TODO : x64/x86 flags
target_link_libraries(ExperimEngine PUBLIC 
	${LIBS_DIR}/lua/lua51.lib
	${LIBS_DIR}/SDL2/x64/SDL2.lib
	${LIBS_DIR}/SDL2/x64/SDL2main.lib
	${Vulkan_LIBRARIES}
)

# Includes.  ${PROJECT_BINARY_DIR} for CMake generated headers
target_include_directories(ExperimEngine PUBLIC
    ${PROJECT_BINARY_DIR}
	${PROJECT_SOURCE_DIR}/includes
)

# Commands after each build
add_custom_command(TARGET ExperimEngine POST_BUILD
	# Binaries
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${LIBS_DIR}/lua/lua51.dll ${CMAKE_BINARY_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${LIBS_DIR}/SDL2/x64/SDL2.dll ${CMAKE_BINARY_DIR}
	# Lua Scripts
	COMMAND ${CMAKE_COMMAND} -E copy_directory  ${PROJECT_SOURCE_DIR}/scripts ${CMAKE_BINARY_DIR}/scripts
)
