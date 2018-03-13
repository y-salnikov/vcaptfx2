cmake_minimum_required (VERSION 2.8)
project (Vcaptfx2)
# The version number.
set (Vcaptfx2_VERSION_MAJOR 1)
set (Vcaptfx2_VERSION_MINOR 0.4)
set (TARGET vcaptfx2)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -pipe -Wall")
#set_directory_properties(PROPERTY ADDITIONAL_MAKE_CLEAN_FILES vcapt_hex.c)

IF(MINGW)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWIN32=1")
ELSE(MINGW)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -DUNIX=1")
ENDIF(MINGW)


set(SOURCE_FILES main.c compat.c usb.c process.c render.c machine.c vcapt_hex.c profile.c)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/Vcaptfx2Config.h.in"
  "${PROJECT_SOURCE_DIR}/Vcaptfx2Config.h"
  )

# add the binary tree to the search path for include files
# so that we will find
include_directories("${PROJECT_BINARY_DIR}")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

find_package(SDL REQUIRED)
if(NOT SDL_FOUND)
    message(SEND_ERROR "Failed to find SDL")
    return()
else()
    include_directories(${SDL_INCLUDE_DIR})
endif()

find_package(libusb-1.0 REQUIRED)
if(NOT LIBUSB_1_FOUND)
	message(SEND_ERROR "Failed to find libusb-1.0")
	return()
else()
    include_directories(${LIBUSB_1_INCLUDE_DIR})
endif()

find_package(libconfig REQUIRED)
if(NOT LIBCONFIG_FOUND)
	message(SEND_ERROR "Failed to find libconfig")
	return()
else()
    include_directories(${LIBCONFIG_INCLUDE_DIR})
endif()

find_package(OpenGL REQUIRED)
if(NOT OPENGL_FOUND)
	message(SEND_ERROR "Failed to find OpenGL")
	return()
else()
    include_directories(${OPENGL_INCLUDE_DIR})
endif()


# add the executable
add_executable(${TARGET} ${SOURCE_FILES} )

target_link_libraries(${TARGET} ${SDL_LIBRARY} ${LIBUSB_1_LIBRARIES} ${LIBCONFIG_LIBRARIES} ${OPENGL_LIBRARIES})



