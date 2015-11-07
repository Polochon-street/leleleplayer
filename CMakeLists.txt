cmake_minimum_required(VERSION 3.1)

project(leleleplayer C)

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
add_executable(leleleplayer WIN32 src/gui.c src/callbacks.c src/gtk_gst_utility.c)
target_link_libraries(leleleplayer ${GTK_LIBRARIES})

foreach(EXE analyze leleleplayer)
	target_link_libraries(${EXE} lelele m)
endforeach ()

if(NOT WIN32)
	set(CMAKE_INSTALL_PREFIX /usr)
endif()
set_property(TARGET lelele analyze leleleplayer PROPERTY C_STANDARD 99)

if(WIN32)
install(TARGETS leleleplayer RUNTIME DESTINATION bin COMPONENT applications)
install(DIRECTORY build_windows/images DESTINATION . COMPONENT applications)
install(DIRECTORY build_windows/lib DESTINATION . COMPONENT applications)
install(DIRECTORY build_windows/bin DESTINATION . COMPONENT applications)
install(DIRECTORY build_windows/share DESTINATION . COMPONENT applications)
install(FILES build_windows/gschemas.compiled DESTINATION . COMPONENT applications)

set(CPACK_PACKAGE_NAME "Leleleplayer")
set(CPACK_PACKAGE_VENDOR "Polochon_street")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "An audio player with a special random function")
set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}\\\\images\\\\lelele.ico")
set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}\\\\images\\\\lelele.ico")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "2")
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "leleleplayer")
set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\leleplayer.exe")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_NSIS_MENU_LINKS "bin\\\\leleleplayer" "Leleleplayer")
set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}\\\\images\\\\lelele.bmp")
# TODO Find a workaround to avoid setting « SetOutPath » manually 
include(CPack)

else()
install(TARGETS leleleplayer DESTINATION bin)
install(FILES "images/lelele.png" "images/lelelerandom.svg" DESTINATION share/leleleplayer/icons/)
install(FILES "org.gnome.leleleplayer.gschema.xml" DESTINATION share/glib-2.0/schemas/)
endif()
