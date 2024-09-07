cmake_minimum_required(VERSION 3.26)
project(
	weno
	VERSION 0.1.0
	LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 					17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS 				OFF)

set(EXECUTABLE_NAME ${PROJECT_NAME})

option(COMPILE_EXECUTABLE "Whether to compile the executable" ON)

add_subdirectory(src)
#add_subdirectory(include)

#if (COMPILE_EXECUTABLE)
#	add_executable(${EXECUTABLE_NAME} ${src}/main.cc)
#else()
#	message("W/o exe. compiling")
#endif()

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake_modules/")
include(AddGitSubmodule)
include(FetchContent)
include(Docs)

add_git_submodule(external/trantor)

add_subdirectory(configured)

FetchContent_Declare(
	trantor
	GIT_REPOSITORY https://github.com/an-tao/trantor
	GIT_TAG v1.5.20
	GIT_SHALLOW TRUE)

FetchContent_MakeAvailable(trantor)

if (COMPILE_EXECUTABLE)
	target_link_libraries(${EXECUTABLE_NAME} PUBLIC trantor)
else()
	message("W/o exe. compiling")
endif()
