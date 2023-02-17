cmake_minimum_required(VERSION 3.10)
project(MainEXE VERSION 1.0)

ADD_SRC_HEADER_FILES()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/SDL/include
	${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/

)

add_executable(MainEXE
	${SRC_FILES}
	${HEADER_FILES}
	${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/glad/glad.c
	${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/glad/KHR/khrplatform.h
	${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/glad/glad/glad.h
)
	
target_include_directories(MainEXE
    PUBLIC ../dependencies
	PUBLIC ../dependencies/glad
)

target_link_libraries(MainEXE
	PUBLIC SDL2
	PUBLIC SDL2main)