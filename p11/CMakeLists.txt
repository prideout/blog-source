CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )
ENABLE_TESTING()

PROJECT( glsw )

FILE( GLOB SOURCE src/*.c src/*.h )
FILE( GLOB TEST_SHADERS test/*.glsl )

INCLUDE_DIRECTORIES( src )

ADD_LIBRARY( glsw ${SOURCE} )
ADD_EXECUTABLE (khash_test test/khash_test.c)
ADD_EXECUTABLE (glsw_test test/glsw_test.c ${TEST_SHADERS} )

TARGET_LINK_LIBRARIES (glsw_test glsw)

ADD_TEST( HashTest khash_test )
ADD_TEST( ShaderTest glsw_test )
