CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )

PROJECT( Viewer )

FILE( GLOB CPP_SOURCE *.hpp *.cpp )
FILE( GLOB C_SOURCE lib/*.h lib/*.c )
FILE( GLOB OPEN_CTM lib/openctm/*.c lib/openctm/*.h )
FILE( GLOB LZMA lib/liblzma/*.c lib/liblzma/*.h )
FILE( GLOB SHADERS *.glsl )

IF( WIN32 )

    SOURCE_GROUP( "lib" FILES ${C_SOURCE} )
    SOURCE_GROUP( "openctm" FILES ${OPEN_CTM} )
    SOURCE_GROUP( "lzma" FILES ${LZMA} )
    ADD_DEFINITIONS( -DGLEW_STATIC )
    SET( PLATFORM_LIBS opengl32 )
    SET( CONSOLE_SYSTEM WIN32 )
    
ELSEIF( APPLE )

    LINK_DIRECTORIES( /usr/X11R6/lib )
    SET( PLATFORM_LIBS X11 GL )

ELSEIF( UNIX )

    SET( PLATFORM_LIBS X11 GL )

ENDIF()

INCLUDE_DIRECTORIES( lib lib/openctm lib/liblzma ${PROJECT_BINARY_DIR} )

ADD_EXECUTABLE( Viewer
    ${CONSOLE_SYSTEM}
    ${CPP_SOURCE}
    ${C_SOURCE}
    ${SHADERS}
    ${LZMA}
    ${OPEN_CTM}
    Platform.hpp
)

TARGET_LINK_LIBRARIES( Viewer ${PLATFORM_LIBS} )
