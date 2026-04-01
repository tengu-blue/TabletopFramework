# Set the minimum CMake version
cmake_minimum_required(VERSION 3.10)

project(03_royal_game_of_ur)

file(COPY ${CMAKE_SOURCE_DIR}/03_royal_game_of_ur/textures DESTINATION ${CMAKE_BINARY_DIR})

add_executable(03_royal_game_of_ur 
	main.cpp
	../cpp.hint
)
target_link_libraries(03_royal_game_of_ur tabletop_frame)

target_include_directories(03_royal_game_of_ur PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/../framework/include
)
