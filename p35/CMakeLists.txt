CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )

PROJECT( CtmViewer )

FILE( GLOB MAIN_SOURCE *.cpp )
FILE( GLOB SHADERS *.glsl )
FILE( GLOB GLEW lib/glew/*.c lib/glew/*.h )
FILE( GLOB GLSW lib/glsw/*.c lib/glsw/*.h )
FILE( GLOB LZMA lib/liblzma/*.c lib/liblzma/*.h )
FILE( GLOB OPENCTM lib/openctm/*.c lib/openctm/*.h )
FILE( GLOB PNGLITE lib/pnglite/*.c lib/pnglite/*.h )
FILE( GLOB VECTORMATH lib/vectormath/cpp/*.h )
FILE( GLOB ZLIB lib/zlib/*.c lib/zlib/*.h )

IF( WIN32 )

    FILE( GLOB PEZ
        lib/libpez/pez.h
        lib/libpez/pez.windows.c
    )

    SOURCE_GROUP( "glew" FILES ${GLEW} )
    SOURCE_GROUP( "glsw" FILES ${GLSW} )
    SOURCE_GROUP( "lzma" FILES ${LZMA} )
    SOURCE_GROUP( "openctm" FILES ${OPENCTM} )
    SOURCE_GROUP( "pnglite" FILES ${PNGLITE} )
    SOURCE_GROUP( "vectormath" FILES ${VECTORMATH} )
    SOURCE_GROUP( "zlib" FILES ${ZLIB} )
    SOURCE_GROUP( "pez" FILES ${PEZ} )
    SOURCE_GROUP( "shaders" FILES ${SHADERS} )

    ADD_DEFINITIONS( -DGLEW_STATIC )
    ADD_DEFINITIONS( -DOPENCTM_STATIC )
    ADD_DEFINITIONS( /wd4996 )
    SET( PLATFORM_LIBS opengl32 )
    SET( CONSOLE_SYSTEM WIN32 )

ELSEIF( APPLE )

    FILE( GLOB PEZ
        lib/libpez/pez.h
        lib/libpez/pez.cocoa.m
    )

    LINK_DIRECTORIES( /usr/X11R6/lib )
    SET( PLATFORM_LIBS X11 GL )

ELSEIF( UNIX )

    FILE( GLOB PEZ
        lib/libpez/pez.h
        lib/libpez/pez.linux.c
    )

    SET( PLATFORM_LIBS X11 GL )

ENDIF()

INCLUDE_DIRECTORIES(
    lib/glew
    lib/glsw
    lib/liblzma
    lib/libpez
    lib/openctm
    lib/pnglite
    lib/vectormath/cpp
    lib/zlib
)

ADD_EXECUTABLE( CtmViewer
    ${CONSOLE_SYSTEM}
    ${MAIN_SOURCE}
    ${SHADERS}
    ${GLEW}
    ${GLSW}
    ${LZMA}
    ${OPENCTM}
    ${PNGLITE}
    ${VECTORMATH}
    ${ZLIB}
    ${PEZ}
)

TARGET_LINK_LIBRARIES( CtmViewer
    ${PLATFORM_LIBS}
)
