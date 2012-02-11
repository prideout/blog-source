CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )

PROJECT( CurlNoise )

FILE( GLOB LIB tinylib/*.c )
FILE( GLOB X11LIB tinylib/pez.x11.c )
FILE( GLOB WINLIB tinylib/pez.win.c )

FILE( GLOB PNGLITE pnglite/*.c )
FILE( GLOB OPENCTM openctm/*.c )
FILE( GLOB MAIN_CPP  *.cpp)
FILE( GLOB MAIN_H    *.hpp)
FILE( GLOB MAIN_GLSL assets/*.glsl )

IF( WIN32 )
    LIST(REMOVE_ITEM LIB ${X11LIB})
ELSE ()
    LIST(REMOVE_ITEM LIB ${X11LIB})
    LIST(REMOVE_ITEM LIB ${WINLIB})
ENDIF ()

ADD_DEFINITIONS( -DGLEW_STATIC -DOPENCTM_STATIC )

INCLUDE_DIRECTORIES(
    tinylib
    openctm
    pnglite
)

ADD_LIBRARY( ThirdParty
    ${LIB}
    ${OPENCTM}
    ${PNGLITE}
)

IF( WIN32 )

    SOURCE_GROUP( "Shaders" FILES ${MAIN_GLSL} )
    SOURCE_GROUP( "Build" FILES CMakeLists.txt )
    ADD_DEFINITIONS( /wd4996 )
    SET( PLATFORM_LIBS opengl32 )
    SET( CONSOLE_SYSTEM WIN32 )

ELSE()

    set(MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME}")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "net.prideout.${PROJECT_NAME}")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_NAME} Version 1.0")
    set(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME})
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING 1.0)
    set(MACOSX_BUNDLE_BUNDLE_VERSION 1.0)
    set(MACOSX_BUNDLE_BUNDLE_ICON "CurlNoise.icns")
    set(MACOSX_BUNDLE_COPYRIGHT "Copyright 2010. All Rights Reserved.")
    
    # These variables are specific to our plist and are NOT standard CMake variables
    set(MACOSX_BUNDLE_NSMAIN_NIB_FILE "MainMenu")
    set(MACOSX_BUNDLE_NSPRINCIPAL_CLASS "NSApplication")

    FIND_PROGRAM(IBTOOL ibtool HINTS "/usr/bin" "${OSX_DEVELOPER_ROOT}/usr/bin")
    IF (${IBTOOL} STREQUAL "IBTOOL-NOTFOUND")
      MESSAGE(SEND_ERROR "ibtool can not be found")
    ENDIF()

    FILE( GLOB COCOA_SOURCE
        cocoa/Pez.Cocoa.m
        cocoa/Controller.m
        cocoa/Controller.h
        cocoa/CustomView.m
        cocoa/CustomView.h
        cocoa/CustomWindow.m
        cocoa/CustomWindow.h
    )

    FIND_LIBRARY( COCOA_LIBRARY Cocoa )

    SET( PLATFORM_LIBS ${COCOA_LIBRARY}
        /System/Library/Frameworks/AppKit.framework
        /System/Library/Frameworks/OpenGL.framework
    )

ENDIF()

ADD_EXECUTABLE( CurlNoise ${CONSOLE_SYSTEM} MACOSX_BUNDLE
	${COCOA_SOURCE}
    ${MAIN_GLSL}
    ${MAIN_CPP}
    ${MAIN_H}
)

TARGET_LINK_LIBRARIES( CurlNoise ThirdParty ${PLATFORM_LIBS} )

if (APPLE)

    SET_TARGET_PROPERTIES(
        CurlNoise PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST
        ../cocoa/Info.plist
    )

    ADD_CUSTOM_COMMAND (
        TARGET CurlNoise
        PRE_BUILD
        COMMENT "Creating Resources folders..."
        COMMAND mkdir -p CurlNoise.app/Contents/Resources
        COMMAND cp ../assets/* CurlNoise.app/Contents/Resources
    )

    ADD_CUSTOM_COMMAND (
        TARGET CurlNoise
        POST_BUILD 
        COMMENT "Building nib for CurlNoise..."
        COMMAND ${IBTOOL} --errors --warnings --notices
        --output-format human-readable-text 
        --compile CurlNoise.app/Contents/Resources/MainMenu.nib 
        ../cocoa/MainMenu.xib
    )
    
ENDIF()