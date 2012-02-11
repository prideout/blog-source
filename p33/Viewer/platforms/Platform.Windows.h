#pragma once

const int ViewportWidth = 384;
const int ViewportHeight = 384;
const bool EnableMultisampling = false;
const bool VerticalSync = false;
const bool OpenGLForwardCompatibility = false;

///////////// Platform-Agnostic Declarations:

const char* GetWindowName();
void Initialize();
void Render();
void Update(unsigned int elapsedMilliseconds);

///////////// Windows Headers and Utility Functions:

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "glew.h"
#include "wglew.h"

void Render(unsigned int elapsedMilliseconds);

#define CheckCondition(A, B) { if (!(A)) { OutputDebugString(B); DebugBreak(); exit(1); } }

inline void DebugStringL(const wchar_t* pStr, ...)
{
    va_list a;
    va_start(a, pStr);

    wchar_t msg[1024] = {0};
    int vsize = _vsnwprintf_s(msg, _countof(msg), _TRUNCATE, pStr, a);
    OutputDebugStringW(msg);
}

inline void DebugString(const char* pStr, ...)
{
    va_list a;
    va_start(a, pStr);

    char msg[1024] = {0};
    int vsize = _vsnprintf_s(msg, _countof(msg), _TRUNCATE, pStr, a);
    OutputDebugStringA(msg);
}

inline void FatalErrorL(const wchar_t* pStr, ...)
{
    va_list a;
    va_start(a, pStr);

    wchar_t msg[1024] = {0};
    int vsize = _vsnwprintf_s(msg, _countof(msg), _TRUNCATE, pStr, a);
    OutputDebugStringW(msg);
    exit(1);
}

inline void FatalError(const char* pStr, ...)
{
    va_list a;
    va_start(a, pStr);

    char msg[1024] = {0};
    int vsize = _vsnprintf_s(msg, _countof(msg), _TRUNCATE, pStr, a);
    OutputDebugStringA(msg);
    exit(1);
}

///////////// Linux/Mac Headers and Utility Functions:

#else

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <wchar.h>
#include "glew.h"
#include "glxew.h"

#define CheckCondition(A, B) { if (!(A)) { fputs(B, stderr); raise(SIGINT); exit(1); } }
#define countof(A) (sizeof(A) / sizeof(A[0]))

inline void DebugStringL(const wchar_t* pStr, ...)
{
    va_list a;
    va_start(a, pStr);

    wchar_t msg[1024] = {0};
    vswprintf(msg, countof(msg), pStr, a);
    fputws(msg, stderr);
}

inline void DebugString(const char* pStr, ...)
{
    va_list a;
    va_start(a, pStr);

    char msg[1024] = {0};
    vsnprintf(msg, countof(msg), pStr, a);
    fputs(msg, stderr);
}

inline void FatalErrorL(const wchar_t* pStr, ...)
{
    fwide(stderr, 1);

    va_list a;
    va_start(a, pStr);

    wchar_t msg[1024] = {0};
    vswprintf(msg, countof(msg), pStr, a);
    fputws(msg, stderr);
    exit(1);
}

inline void FatalError(const char* pStr, ...)
{
    va_list a;
    va_start(a, pStr);

    char msg[1024] = {0};
    vsnprintf(msg, countof(msg), pStr, a);
    fputs(msg, stderr);
    exit(1);
}

#endif