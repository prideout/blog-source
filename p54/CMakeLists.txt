CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )

PROJECT( Silhouette )

FILE( GLOB GLEW lib/glew/*.c )
FILE( GLOB GLSW lib/glsw/*.c )
FILE( GLOB OPENCTM lib/openctm/*.c )
FILE( GLOB LIBLZMA lib/liblzma/*.c )
FILE( GLOB JUDY lib/judy/JudyL/*.c lib/judy/JudyCommon/JudyMalloc.c )

ADD_DEFINITIONS( -DGLEW_STATIC -DOPENCTM_STATIC -DJUDYL )
ADD_DEFINITIONS( -std=c99 )

IF( WIN32 )
    FILE( GLOB PEZ win/Main.c )
    SET( PLATFORM_LIBS opengl32 )
    SET( CONSOLE_SYSTEM WIN32 )

ELSEIF( UNIX )
    FILE( GLOB PEZ Linux.c )
    SET( PLATFORM_LIBS X11 GL )
ENDIF()

INCLUDE_DIRECTORIES(
    lib/glew
    lib/glsw
    lib/judy
    lib/judy/JudyCommon
    lib/liblzma
    lib/openctm
    lib/vectormath
)

ADD_LIBRARY( Ecosystem
    ${GLEW}
    ${GLSW}
    ${VECTORMATH}
    ${OPENCTM}
    ${LIBLZMA}
    ${JUDY}
    ${PEZ}
)

ADD_EXECUTABLE( Silhouette ${CONSOLE_SYSTEM}
    Adjacency.c
    Utility.h
    Platform.h
    CreateMesh.c
    CreateProgram.c
    Silhouette.c
    Silhouette.glsl )
    
TARGET_LINK_LIBRARIES( Silhouette Ecosystem ${PLATFORM_LIBS} )
