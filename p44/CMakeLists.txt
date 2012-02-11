CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )

PROJECT( Wireframe )

FILE( GLOB GLEW lib/glew/*.c lib/glew/*.h )
FILE( GLOB GLSW lib/glsw/*.c lib/glsw/*.h )
FILE( GLOB LZMA lib/liblzma/*.c lib/liblzma/*.h )
FILE( GLOB OPENCTM lib/openctm/*.c lib/openctm/*.h )
FILE( GLOB VECTORMATH lib/vectormath/c/*.h )

IF( WIN32 )

    FILE( GLOB PEZ
        lib/libpez/pez.h
        lib/libpez/pez.windows.c
    )

    SOURCE_GROUP( "shaders" FILES Wireframe.glsl )
    SOURCE_GROUP( "meshes" FILES sphere.ctm )
    SOURCE_GROUP( "glew" FILES ${GLEW} )
    SOURCE_GROUP( "glsw" FILES ${GLSW} )
    SOURCE_GROUP( "lzma" FILES ${LZMA} )
    SOURCE_GROUP( "openctm" FILES ${OPENCTM} )
    SOURCE_GROUP( "vectormath" FILES ${VECTORMATH} )
    SOURCE_GROUP( "pez" FILES ${PEZ} )

    ADD_DEFINITIONS( -DGLEW_STATIC )
    ADD_DEFINITIONS( -DOPENCTM_STATIC )
    ADD_DEFINITIONS( /wd4996 )
    SET( PLATFORM_LIBS opengl32 )
    SET( CONSOLE_SYSTEM WIN32 )

ELSEIF( APPLE )

    # Create a nibless project per http://blog.hyperjeff.net/code?id=292
    
    FILE( GLOB PEZ
        lib/libpez/pez.h
        lib/libpez/pez.cocoa.m
    )

    FIND_LIBRARY( COCOA_LIBRARY Cocoa )
    
    SET( PLATFORM_LIBS
        ${COCOA_LIBRARY}
        /System/Library/Frameworks/OpenGL.framework )

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
)

ADD_LIBRARY( PezEcosystem
    ${GLEW}
    ${GLSW}
    ${LZMA}
    ${OPENCTM}
    ${VECTORMATH}
    ${PEZ}
)

INCLUDE_DIRECTORIES( lib/vectormath/c )
ADD_EXECUTABLE( Wireframe ${CONSOLE_SYSTEM} Wireframe.c Wireframe.glsl sphere.ctm )
TARGET_LINK_LIBRARIES( Wireframe PezEcosystem ${PLATFORM_LIBS} )
