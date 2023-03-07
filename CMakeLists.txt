cmake_minimum_required(VERSION 3.10)


project(OpenGLProgram VERSION 1.0)

macro(ADD_SRC_HEADER_FILES )
	file(GLOB_RECURSE SRC_FILES LIST_DIRECTORIES false RELATIVE
		${CMAKE_CURRENT_SOURCE_DIR} *.c??)
	file(GLOB_RECURSE HEADER_FILES LIST_DIRECTORIES false RELATIVE
		${CMAKE_CURRENT_SOURCE_DIR} *.h)    
endmacro()

find_package(Vulkan REQUIRED)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

message("Using SDL2...")
add_subdirectory(src/dependencies/SDL)
option(ASSIMP_BUILD_ZLIB "Build the static library" ON)
add_subdirectory(src/dependencies/assimp)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/dependencies/SDL)


add_subdirectory(src/MainEXE)

