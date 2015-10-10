cmake_minimum_required(VERSION 3.1)

project(leleleplayer C)
include(CPack)
find_package(PkgConfig REQUIRED)

if(WIN32)
pkg_check_modules(MULTIMEDIA REQUIRED libavformat libavutil libavcodec gstreamer-1.0 gstreamer-video-1.0)
else()
pkg_check_modules(MULTIMEDIA REQUIRED libavformat libavutil libavcodec gstreamer-1.0 gstreamer-video-1.0 x11)
endif()

pkg_check_modules(GTK REQUIRED gtk+-3.0)

include_directories(${MULTIMEDIA_INCLUDE_DIRS} ${GTK_INCLUDE_DIRS})
link_directories(${MULTIMEDIA_LIBRARY_DIRS} ${GTK_LIBRARY_DIRS})
add_definitions(${MULTIMEDIA_CFLAGS_OTHER} ${GTK_CFLAGS_OTHER})

ADD_LIBRARY(lelele src/amp_sort.c src/decode.c src/envelope.c src/freq_sort.c src/analyze.c)
target_link_libraries(lelele ${MULTIMEDIA_LIBRARIES})

add_executable(analyze src/gui_dummy.c)
add_executable(leleleplayer src/gui.c src/gtk_gst_utility.c) #win32
target_link_libraries(leleleplayer ${GTK_LIBRARIES})

foreach(EXE analyze leleleplayer)
	target_link_libraries(${EXE} lelele m)
endforeach ()

set_property(TARGET lelele analyze leleleplayer PROPERTY C_STANDARD 99)
install(TARGETS leleleplayer DESTINATION bin)
install(FILES "images/lelele.svg" "images/lelelerandom.svg" DESTINATION /usr/share/leleleplayer/icons/)
install(FILES "org.leleleplayer.gschema.xml" DESTINATION /usr/share/glib-2.0/schemas/)
