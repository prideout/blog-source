CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )
PROJECT( Fluid3D )
FILE( GLOB LIB *.c *.cpp *.h *.hpp )
FILE( GLOB MAIN Utility.* Fluid3D.cpp Trackball.cpp *.glsl )
LIST(REMOVE_ITEM LIB ${MAIN})
ADD_DEFINITIONS( -DGLEW_STATIC /wd4996 )
INCLUDE_DIRECTORIES( . )
ADD_LIBRARY( Ecosystem ${LIB} )
SOURCE_GROUP( "Build" FILES CMakeLists.txt )
SOURCE_GROUP( "Source Files" FILES Raycast.glsl Fluid.glsl )
SET( PLATFORM_LIBS opengl32 )
SET( CONSOLE_SYSTEM WIN32 )
ADD_EXECUTABLE( Fluid3D ${CONSOLE_SYSTEM} ${MAIN} )
TARGET_LINK_LIBRARIES( Fluid3D Ecosystem ${PLATFORM_LIBS} )
