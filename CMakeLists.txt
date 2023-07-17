cmake_minimum_required(VERSION 3.5)

project(mstream LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(USE_TENGINE "Use Tengine library" ON)
option(USE_RKNN "Use RKNN library" OFF)

EXECUTE_PROCESS(COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCH)
message(STATUS "Arch: ${ARCH}")

if (${ARCH} MATCHES "armv7l")
	add_definitions(-DARM7l)
elseif (${ARCH} MATCHES "aarch64")
	add_definitions(-DAARCH64)
else()
	add_definitions(-DX86_64)
endif()

# Threads
find_package(Threads REQUIRED)

# Json
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
link_libraries(${JSONCPP_LIBRARIES})

# OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_DIRS})

# Gstreamer
find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED IMPORTED_TARGET gstreamer-1.0)

include_directories(
	${GLIB_INCLUDE_DIRS}
	${GSTREAMER_INCLUDE_DIRS}
	)

# LibAV
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
	libavformat
	libavcodec
	libswresample
	libswscale
	libavutil
	)

# RKNN
if (USE_RKNN)
	add_definitions(-DUSE_RKNN)
	if (${ARCH} MATCHES "aarch64")
		set(RKNN_RT_LIB ${CMAKE_SOURCE_DIR}/lib/rknn_api/arm/aarch64/librknnrt.so)
		include_directories("lib/rknn_api/arm/include")
	else()
		set(RKNN_RT_LIB ${CMAKE_SOURCE_DIR}/lib/rknn_api/x86_64/lib64/librknn_api.so)
		include_directories("lib/rknn_api/x86_64/include")
	endif()
	link_libraries(${RKNN_RT_LIB})
	message(STATUS "Rknn: ${RKNN_RT_LIB}")
endif()

# Tengine
if (USE_TENGINE)
	add_definitions(-DUSE_TENGINE)
	if (${ARCH} MATCHES "aarch64")
		set(TENGINE_RT_LIB ${CMAKE_SOURCE_DIR}/lib/tengine/aarch64/libtengine-lite.so)
		include_directories("lib/tengine/include")
	elseif (${ARCH} MATCHES "armv7l")
		set(TENGINE_RT_LIB ${CMAKE_SOURCE_DIR}/lib/tengine/armv7l/libtengine-lite.so)
		include_directories("lib/tengine/include")
	else()
		set(TENGINE_RT_LIB ${CMAKE_SOURCE_DIR}/lib/tengine/x86_64/libtengine-lite.so)
		include_directories("lib/tengine/include")
	endif()
	link_libraries(${TENGINE_RT_LIB})
	message(STATUS "Tengine: ${TENGINE_RT_LIB}")
endif()

# Common group
file (GLOB COMMON_SRC "src/common/*.cpp" "src/common/*.h")
source_group("common" FILES ${COMMON_SRC})
set (SOURCES ${SOURCES} ${COMMON_SRC})

# Main group
file (GLOB CORE_SRC "src/main/*.cpp" "src/main/*.h")
source_group("main" FILES ${CORE_SRC})
set (SOURCES ${SOURCES} ${CORE_SRC})

# NN group
file (GLOB NN_SRC "src/nn/*.cpp" "src/nn/*.h" "src/nn/*.cc"
	"src/nn/rknn/*.cpp" "src/nn/rknn/*.h" "src/nn/rknn/*.cc"
	"src/nn/tengine/*.cpp" "src/nn/tengine/*.h" "src/nn/tengine/*.cc")
source_group("nn" FILES ${NN_SRC})
set (SOURCES ${SOURCES} ${NN_SRC})

# Input group
file (GLOB INPUT_SRC "src/input/*.cpp" "src/input/*.h")
source_group("input" FILES ${INPUT_SRC})
set (SOURCES ${SOURCES} ${INPUT_SRC})

# Output group
file (GLOB OUTPUT_SRC "src/output/*.cpp" "src/output/*.h")
source_group("output" FILES ${OUTPUT_SRC})
set (SOURCES ${SOURCES} ${OUTPUT_SRC})

# Imgproc group
file (GLOB IMGPROC_SRC "src/imgproc/*.cpp" "src/imgproc/*.h")
source_group("imgproc" FILES ${IMGPROC_SRC})
set (SOURCES ${SOURCES} ${IMGPROC_SRC})

# Config
file(GLOB CONFIGFILES "data/*.json")
source_group("config" FILES ${CONFIGFILES})
set (SOURCES ${SOURCES} ${CONFIGFILES})

# Sources
include_directories("src")

# Copy file for tim-vx
file(COPY "src/vx/cl_viv_vx_ext.h" DESTINATION ${PROJECT_BINARY_DIR})

# For debug with tim-vx
set(ENV{DISABLE_IDE_DEBUG} 1)
#execute_process(COMMAND export "DISABLE_IDE_DEBUG=1")

# Executable
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} Threads::Threads -lfreeimage ${JSONCPP_LIBRARIES} ${OpenCV_LIBS} PkgConfig::GSTREAMER PkgConfig::LIBAV)

