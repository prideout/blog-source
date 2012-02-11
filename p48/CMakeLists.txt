CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )

PROJECT( Tessellation )

FILE( GLOB GLEW lib/glew/*.c lib/glew/*.h )
FILE( GLOB GLSW lib/glsw/*.c lib/glsw/*.h )
FILE( GLOB VECTORMATH lib/vectormath/*.h )

IF( WIN32 )

    FILE( GLOB PEZ
        lib/libpez/pez.h
        lib/libpez/pez.windows.c
    )

    SOURCE_GROUP( "shaders" FILES Geodesic.glsl )
    SOURCE_GROUP( "glew" FILES ${GLEW} )
    SOURCE_GROUP( "glsw" FILES ${GLSW} )
    SOURCE_GROUP( "vectormath" FILES ${VECTORMATH} )
    SOURCE_GROUP( "pez" FILES ${PEZ} )

    ADD_DEFINITIONS( -DGLEW_STATIC )
    ADD_DEFINITIONS( /wd4996 /TP )
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
    lib/libpez
)

ADD_LIBRARY( PezEcosystem
    ${GLEW}
    ${GLSW}
    ${VECTORMATH}
    ${PEZ}
)

INCLUDE_DIRECTORIES( lib/vectormath )
ADD_EXECUTABLE( Geodesic ${CONSOLE_SYSTEM} Geodesic.c Geodesic.glsl )
TARGET_LINK_LIBRARIES( Geodesic PezEcosystem ${PLATFORM_LIBS} )
