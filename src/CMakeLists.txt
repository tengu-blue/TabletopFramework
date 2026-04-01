cmake_minimum_required(VERSION 3.11)

project(TabletopFramework)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find OpenGL package
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

# for pulling
function(fetch_glfw) 
    include(FetchContent)

	# Use FetchContent to download and configure GLFW
	FetchContent_Declare(
		glfw
		GIT_REPOSITORY https://github.com/glfw/glfw.git
		GIT_TAG        3.4
	)

	FetchContent_GetProperties(glfw)
	if(NOT glfw_POPULATED)
		FetchContent_Populate(glfw)

		set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
		set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
		set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
		set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")

		add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR})

		message("GLFW source and binary directories:" ${glfw_SOURCE_DIR} " " ${glfw_BINARY_DIR})

	endif()
endfunction()

function(fetch_glm) 

	include(FetchContent)

	FetchContent_Declare(
			glm
			GIT_REPOSITORY	https://github.com/g-truc/glm.git
			GIT_TAG 	bf71a834948186f4097caa076cd2663c69a10e1e #refs/tags/1.0.1
	)

	FetchContent_MakeAvailable(glm)

	message("GLM source directory:" ${glm_SOURCE_DIR})

endfunction()

if(WIN32)

	find_path(GLFW_ROOT_DIR NAMES include/GLFW/glfw3.h
		PATHS
			"${CMAKE_CURRENT_SOURCE_DIR}/libs/glfw-3.4.bin.WIN64"
			"${CMAKE_CURRENT_SOURCE_DIR}/libs/glfw-3.4"
			"${CMAKE_CURRENT_SOURCE_DIR}/../libs/glfw-3.4.bin.WIN64"
			"${CMAKE_CURRENT_SOURCE_DIR}/../libs/glfw-3.4"
		NO_DEFAULT_PATH) # Avoid searching in default paths

	if(GLFW_ROOT_DIR)
		message("GLFW_ROOT_DIR is: " ${GLFW_ROOT_DIR})
		# Determine the appropriate library directory based on the Visual Studio version
		if(MSVC_VERSION GREATER_EQUAL 1930 AND MSVC_VERSION LESS 1950)
			set(GLFW_LIB_DIR "${GLFW_ROOT_DIR}/lib-vc2022")
		elseif(MSVC_VERSION GREATER_EQUAL 1920 AND MSVC_VERSION LESS 1930)
			set(GLFW_LIB_DIR "${GLFW_ROOT_DIR}/lib-vc2019")
		elseif(MSVC_VERSION GREATER_EQUAL 1910 AND MSVC_VERSION LESS 1920)
			set(GLFW_LIB_DIR "${GLFW_ROOT_DIR}/lib-vc2017")
			# Add more conditions for other Visual Studio versions as needed
		else()
			message(FATAL_ERROR "Unsupported Visual Studio version for GLFW.")
		endif()

		message("GLFW lib directory: " ${GLFW_LIB_DIR})
		add_library(glfw INTERFACE)

		# Set the include directory for the glfw target
		target_include_directories(glfw INTERFACE "${GLFW_ROOT_DIR}/include")

		# Assuming the GLFW library filename; adjust as necessary
		find_library(GLFW_LIBRARY NAMES glfw3 PATHS "${GLFW_LIB_DIR}" NO_DEFAULT_PATH)

		if(NOT GLFW_LIBRARY)
			message(FATAL_ERROR "GLFW library not found.") 
		endif()

		# Link the library with the glfw target
		target_link_libraries(glfw INTERFACE "${GLFW_LIBRARY}")

		# Specify the include directory
		set(GLFW_INCLUDE_DIR "${GLFW_ROOT_DIR}/include")

	else()
		FETCH_GLFW()
	endif()


	find_path(GLM_ROOT_DIR NAMES glm/glm.hpp
		PATHS
			"${CMAKE_CURRENT_SOURCE_DIR}/libs/glm-1.0.1"
			"${CMAKE_CURRENT_SOURCE_DIR}/../libs/glm-1.0.1"
		NO_DEFAULT_PATH)

	if(NOT GLM_ROOT_DIR) 
		message("GLM not found, trying to fetch")
		# Use FetchContent to download and configure GLM
		FETCH_GLM()
	else()
		message("GLM_ROOT_DIR is: " ${GLM_ROOT_DIR})

		add_library(glm INTERFACE)
		target_include_directories(glm INTERFACE "${GLM_ROOT_DIR}")
		target_compile_definitions(glm INTERFACE GLM_ENABLE_EXPERIMENTAL GLM_FORCE_SWIZZLE)
		add_library(glm::glm ALIAS glm)
	endif()

	
else()

	# try to look for them else fetch them
	include(FetchContent)


	find_package(glfw3 3.4 QUIET)
	if(NOT glfw3_FOUND)
		message("GLFW not found, trying to fetch")
		# Use FetchContent to download and configure GLFW
		FETCH_GLFW()
	endif()


	find_package(glm QUIET)
	if(NOT glm_FOUND)
		message("GLM not found, trying to fetch")
		# Use FetchContent to download and configure GLM
		FETCH_GLM()
	else()
		add_compile_definitions("GLM_FORCE_SWIZZLE")
	endif()

endif()


# shaders
file(COPY ${CMAKE_SOURCE_DIR}/framework/shaders DESTINATION ${CMAKE_BINARY_DIR})

# framework --------------------------------------------------------------------------------
file(GLOB FRAMEWORK_SOURCE_FILES "framework/src/*.cpp")
set(FRAMEWORK_HEADER_FILES "framework/include")

message("Framework sources: " )

# Create a library with public headers
add_library(tabletop_frame ${FRAMEWORK_HEADER_FILES} ${FRAMEWORK_SOURCE_FILES})
target_sources(tabletop_frame PRIVATE glad/src/glad.c)
target_link_libraries(tabletop_frame glm::glm glfw OpenGL::GL)
target_include_directories(tabletop_frame PUBLIC
	glad/include
	stb
	utilities
	framework/include
)

# framework --------------------------------------------------------------------------------


# actual projects
add_subdirectory(03_royal_game_of_ur)
