#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <glew.h>

#define TwoPi (6.28318531f)
#define Pi (3.14159265f)
#define countof(A) (sizeof(A) / sizeof(A[0]))

// Implemented by the demo:
const char* PezInitialize(int width, int height); // receive window size and return window title
void PezRender(unsigned int fbo);                 // draw scene (Pez swaps the backbuffer for you)
void PezUpdate(unsigned int microseconds);        // receive elapsed time (e.g., update physics)
void PezHandleMouse(int x, int y, int action);    // handle mouse action specified by bitfield

enum MouseFlag
{
    PEZ_DOWN  = 1 << 0,
    PEZ_UP    = 1 << 1,
    PEZ_MOVE  = 1 << 2,
    PEZ_LEFT  = 1 << 3,
    PEZ_RIGHT = 1 << 4,
    PEZ_DOUBLECLICK = 1 << 5,
};

// Implemented by the platform layer:
const char* PezResourcePath();
void PezDebugString(const char* pStr, ...);
void PezDebugStringW(const wchar_t* pStr, ...);
void PezFatalError(const char* pStr, ...);
void PezFatalErrorW(const wchar_t* pStr, ...);
void PezCheckCondition(int condition, ...);
void PezCheckConditionW(int condition, ...);
int PezIsPressing(char key);

// Configuration:
#define PEZ_VIEWPORT_WIDTH (853)
#define PEZ_VIEWPORT_HEIGHT (480)
#define PEZ_ENABLE_MULTISAMPLING 0
#define PEZ_VERTICAL_SYNC 0
#define PEZ_FORWARD_COMPATIBLE_GL 0

#ifdef __cplusplus
}
#endif
