PROJECT(SIMON C)

cmake_minimum_required(VERSION 2.6)
find_package( PkgConfig REQUIRED )

#set(CMAKE_C_FLAGS "-g -Wall")
IF((NOT DEFINED CMAKE_C_FLAGS) OR (CMAKE_C_FLAGS STREQUAL ""))
	SET (CMAKE_C_FLAGS "-Os -Wall -g")
ENDIF()

# External packages we need
pkg_check_modules (Libgtop2   libgtop-2.0)
pkg_check_modules (GTK2   gtk+-2.0)
pkg_check_modules (GOBJECT   gobject-2.0)


link_directories (
	${GTK2_LIBRARY_DIRS}
	${GOBJECT_LIBRARY_DIRS}
	${Libgtop2_LIBRARY_DIRS}
)

include_directories (
	${GTK2_INCLUDE_DIRS}
	${GOBJECT_INCLUDE_DIRS}
	${Libgtop2_INCLUDE_DIRS}
	$(SIMON_SOURCE_DIR)/src/
)

add_subdirectory(src)

add_executable(simon ${simonSources} )
target_link_libraries(simon  ${GTK2_LIBRARIES} ${GOBJECT_LIBRARIES} ${Libgtop2_LIBRARIES} m)
