CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )

PROJECT( Splat )

FILE( GLOB GLEW lib/glew/*.c )
FILE( GLOB GLSW lib/glsw/*.c )

ADD_DEFINITIONS( -DGLEW_STATIC -DOPENCTM_STATIC )

IF( WIN32 )
    FILE( GLOB PEZ lib/pez/Windows.c )
    SET( PLATFORM_LIBS opengl32 )
    SET( CONSOLE_SYSTEM WIN32 )

ELSEIF( UNIX )
    FILE( GLOB PEZ lib/pez/Linux.c )
    SET( PLATFORM_LIBS X11 GL )
ENDIF()

INCLUDE_DIRECTORIES(
    lib/glew
    lib/glsw
    lib/pez
    lib/vectormath
)

ADD_LIBRARY( Ecosystem
    ${GLEW}
    ${GLSW}
    ${VECTORMATH}
    ${PEZ}
)

FILE( GLOB MAIN_CPP  *.cpp )
FILE( GLOB MAIN_H    *.h )
FILE( GLOB MAIN_GLSL *.glsl )

ADD_EXECUTABLE( Splat ${CONSOLE_SYSTEM}
    ${MAIN_GLSL}
    ${MAIN_CPP}
    ${MAIN_H} )
    
TARGET_LINK_LIBRARIES( Splat Ecosystem ${PLATFORM_LIBS} )
