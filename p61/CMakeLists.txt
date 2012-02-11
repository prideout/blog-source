CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )

PROJECT( Tron )

FILE( GLOB LIB tinylib/*.* )
FILE( GLOB WINLIB Text.cpp tinylib/pez.win.c )

FILE( GLOB MAIN_CPP  *.cpp)
FILE( GLOB WIN_CPP   Text.cpp)
FILE( GLOB MAIN_H    *.hpp)
FILE( GLOB MAIN_GLSL *.glsl )

IF (APPLE)
    LIST(REMOVE_ITEM LIB      ${WINLIB})
    LIST(REMOVE_ITEM MAIN_CPP ${WIN_CPP})
ENDIF ()

ADD_DEFINITIONS( -DGLEW_STATIC )

INCLUDE_DIRECTORIES(
    tinylib
)

ADD_LIBRARY( ThirdParty
    ${LIB}
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
    set(MACOSX_BUNDLE_BUNDLE_ICON "Tron.icns")
    set(MACOSX_BUNDLE_COPYRIGHT "Copyright 2010. All Rights Reserved.")
    
    # These variables are specific to our plist and are NOT standard CMake variables
    set(MACOSX_BUNDLE_NSMAIN_NIB_FILE "MainMenu")
    set(MACOSX_BUNDLE_NSPRINCIPAL_CLASS "NSApplication")

    FIND_PROGRAM(IBTOOL ibtool HINTS "/usr/bin" "${OSX_DEVELOPER_ROOT}/usr/bin")
    IF (${IBTOOL} STREQUAL "IBTOOL-NOTFOUND")
      MESSAGE(SEND_ERROR "ibtool can not be found")
    ENDIF()

    FILE( GLOB COCOA_SOURCE
        cocoa/Text.mm
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

ADD_EXECUTABLE( Tron ${CONSOLE_SYSTEM} MACOSX_BUNDLE
	${COCOA_SOURCE}
    ${MAIN_GLSL}
    ${MAIN_CPP}
    ${MAIN_H}
)

TARGET_LINK_LIBRARIES( Tron ThirdParty ${PLATFORM_LIBS} )

if (APPLE)

    SET_TARGET_PROPERTIES(
        Tron PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST
        ../cocoa/Info.plist
    )

    ADD_CUSTOM_COMMAND (
        TARGET Tron
        PRE_BUILD
        COMMENT "Creating Resources folders..."
        COMMAND mkdir -p Tron.app/Contents/Resources
        COMMAND cp ../*.glsl Tron.app/Contents/Resources
        COMMAND cp ../*.dds Tron.app/Contents/Resources
    )

    ADD_CUSTOM_COMMAND (
        TARGET Tron
        POST_BUILD 
        COMMENT "Building nib for Tron..."
        COMMAND ${IBTOOL} --errors --warnings --notices
        --output-format human-readable-text 
        --compile Tron.app/Contents/Resources/MainMenu.nib 
        ../cocoa/MainMenu.xib
    )
    
ENDIF()