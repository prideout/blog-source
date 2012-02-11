#pragma once
#include <stdbool.h>
#include <glew.h>

enum {PEZ_DOWN, PEZ_UP, PEZ_MOVE};
#define TwoPi (6.28318531f)
#define Pi (3.14159265f)
#define countof(A) (sizeof(A) / sizeof(A[0]))

// Implemented by the demo:
const char* PezInitialize(int width, int height); // receive window size and return window title
void PezRender(unsigned int fbo);                 // draw scene (Pez swaps the backbuffer for you)
void PezUpdate(unsigned int microseconds);        // receive elapsed time (e.g., update physics)
void PezHandleMouse(int x, int y, int action);    // handle mouse action: PEZ_DOWN, PEZ_UP, or PEZ_MOVE

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
#define PEZ_VIEWPORT_WIDTH (853*2)
#define PEZ_VIEWPORT_HEIGHT (480*2)
//#define PEZ_VIEWPORT_WIDTH 853
//#define PEZ_VIEWPORT_HEIGHT 480
//#define PEZ_VIEWPORT_WIDTH 768
//#define PEZ_VIEWPORT_HEIGHT 768
#define PEZ_ENABLE_MULTISAMPLING 1
#define PEZ_VERTICAL_SYNC 0
#define PEZ_FORWARD_COMPATIBLE_GL 0
