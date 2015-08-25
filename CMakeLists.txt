cmake_minimum_required(VERSION 3.1)

project(leleleplayer C)

find_package(PkgConfig REQUIRED)

if(WIN32)
pkg_check_modules(MULTIMEDIA REQUIRED libavformat libavutil libavcodec gstreamer-1.0 gstreamer-video-1.0)
ELSE()
pkg_check_modules(MULTIMEDIA REQUIRED libavformat libavutil libavcodec gstreamer-1.0 gstreamer-video-1.0 x11)
ENDIF()
pkg_check_modules(GTK REQUIRED gtk+-3.0)

include_directories(${MULTIMEDIA_INCLUDE_DIRS} ${GTK_INCLUDE_DIRS})
link_directories(${MULTIMEDIA_LIBRARY_DIRS} ${GTK_LIBRARY_DIRS})
add_definitions(${MULTIMEDIA_CFLAGS_OTHER} ${GTK_CFLAGS_OTHER})

ADD_LIBRARY(lelele src/amp_sort.c src/decode.c src/envelope.c src/freq_sort.c src/analyze.c)
target_link_libraries(lelele ${MULTIMEDIA_LIBRARIES})

#if(WIN32)
#SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
#SET(BUILD_SHARED_LIBRARIES OFF)
#SET(CMAKE_EXE_LINKER_FLAGS "-static")
#endif()
add_executable(analyze src/gui_dummy.c)
add_executable(player src/gui.c) #win32
target_link_libraries(player ${GTK_LIBRARIES})

foreach (EXE analyze player)
	target_link_libraries(${EXE} lelele m)
endforeach ()

set_property(TARGET lelele analyze player PROPERTY C_STANDARD 99)

