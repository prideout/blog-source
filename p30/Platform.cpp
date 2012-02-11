#include "Platform.hpp"
#include "Vector.hpp"

#ifdef WIN32

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
    LPCSTR szName = GetWindowName();
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, GetModuleHandle(0), 0, 0, 0, 0, szName, 0 };
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    RegisterClassEx(&wc);

    DWORD dwStyle = WS_SYSMENU | WS_VISIBLE | WS_POPUP;
    DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;

    RECT rect;
    SetRect(&rect, 0, 0, ViewportWidth, ViewportHeight);
    AdjustWindowRectEx(&rect, dwStyle, FALSE, dwExStyle);
    int windowWidth = rect.right - rect.left;
    int windowHeight = rect.bottom - rect.top;
    int windowLeft = GetSystemMetrics(SM_CXSCREEN) / 2 - windowWidth / 2;
    int windowTop = GetSystemMetrics(SM_CYSCREEN) / 2 - windowHeight / 2;

    HWND hWnd = CreateWindowExA(0, szName, szName, dwStyle, windowLeft, windowTop, windowWidth, windowHeight, 0, 0, 0, 0);

    // Create the GL context.
    PIXELFORMATDESCRIPTOR pfd;
    ZeroMemory(&pfd, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    HDC hDC = GetDC(hWnd);
    int pixelFormat = ChoosePixelFormat(hDC, &pfd);

    SetPixelFormat(hDC, pixelFormat, &pfd);
    HGLRC hRC = wglCreateContext(hDC);
    wglMakeCurrent(hDC, hRC);

    if (EnableMultisampling)
    {
        PROC proc = wglGetProcAddress("wglChoosePixelFormatARB");
        PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC) proc;
        if (!wglChoosePixelFormatARB)
        {
            FatalError("Could not load function pointer for 'wglChoosePixelFormatARB'.  Is your driver properly installed?");
        }

        unsigned int numFormats;
        int pixelAttribs[] =
        {
            WGL_SAMPLES_ARB, 16,
            WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
            WGL_RED_BITS_ARB, 8,
            WGL_GREEN_BITS_ARB, 8,
            WGL_BLUE_BITS_ARB, 8,
            WGL_ALPHA_BITS_ARB, 8,
            WGL_DEPTH_BITS_ARB, 24,
            WGL_STENCIL_BITS_ARB, 8,
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
            0
        };

        int* sampleCount = pixelAttribs + 1;
        int* useSampleBuffer = pixelAttribs + 3;
        int pixelFormat = -1;

        // Try fewer and fewer samples per pixel till we find one that is supported:
        while (pixelFormat <= 0 && *sampleCount >= 0)
        {
            wglChoosePixelFormatARB(hDC, pixelAttribs, 0, 1, &pixelFormat, &numFormats);
            (*sampleCount)--;
            if (*sampleCount <= 1)
            {
                *useSampleBuffer = GL_FALSE;
            }
        }

        // Win32 allows the pixel format to be set only once per app, so destroy and re-create the app:
        DestroyWindow(hWnd);
        hWnd = CreateWindowExA(0, szName, szName, dwStyle, windowLeft, windowTop, windowWidth, windowHeight, 0, 0, 0, 0);
        SetWindowPos(hWnd, HWND_TOP, windowLeft, windowTop, windowWidth, windowHeight, 0);
        hDC = GetDC(hWnd);
        SetPixelFormat(hDC, pixelFormat, &pfd);
        hRC = wglCreateContext(hDC);
        wglMakeCurrent(hDC, hRC);
    }

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        FatalError("GLEW Error: %s\n", glewGetErrorString(err));
    }
    DebugString("OpenGL Version: %s\n", glGetString(GL_VERSION));

    if (!VerticalSync)
    {
        wglSwapIntervalEXT(0);
    }

    if (OpenGLForwardCompatibility && glewIsSupported("GL_VERSION_3_0"))
    {
        const int contextAttribs[] =
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 0,
            WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
            0
        };

        HGLRC newRC = wglCreateContextAttribsARB(hDC, 0, contextAttribs);
        wglMakeCurrent(0, 0);
        wglDeleteContext(hRC);
        hRC = newRC;
        wglMakeCurrent(hDC, hRC);
    }

    Initialize();

    // -------------------
    // Start the Game Loop
    // -------------------

    DWORD waitTime = 0;
    DWORD previousTime = GetTickCount();
    MSG msg = {0};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            if (glGetError() != GL_NO_ERROR)
            {
                FatalError("OpenGL error.\n");
            }

            DWORD currentTime = GetTickCount();
            DWORD deltaTime = currentTime - previousTime;
            previousTime = currentTime;

            Update(deltaTime);

            Render();
            SwapBuffers(hDC);
        }
    }

    UnregisterClass(szName, wc.hInstance);

    return 0;
}

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    ivec2 p(LOWORD(lParam), HIWORD(lParam));
    switch (msg)
    {
        case WM_LBUTTONUP:
            break;

        case WM_LBUTTONDOWN:
            break;

        case WM_MOUSEMOVE:
            break;

        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_ESCAPE:
                    PostQuitMessage(0);
                    break;
                case VK_OEM_2: // Question Mark / Forward Slash for US Keyboards
                    break;
            }
            break;
        }
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

#else // not WIN32

#include <sys/time.h>
#include <iostream>
#include <string.h>

using namespace std;

struct PlatformContext
{
    Display* MainDisplay;
    Window MainWindow;
};

unsigned int GetMilliseconds()
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

void PrettyPrintExtensions()
{
    string extensions = (const char*) glGetString(GL_EXTENSIONS);
    char* extensionStart = &extensions[0];
    char** extension = &extensionStart;
    cout << "Supported OpenGL Extensions:" << endl;
    while (*extension) cout << '\t' << strsep(extension, " ") << endl;
    cout << endl;
}

int main(int argc, char** argv)
{
    int attrib[] =
    {
        GLX_RGBA,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_DEPTH_SIZE, 24,
        GLX_DOUBLEBUFFER, None,
    };
    
    PlatformContext context;

    context.MainDisplay = XOpenDisplay(NULL);
    int screen = DefaultScreen(context.MainDisplay);
    Window root = RootWindow(context.MainDisplay, screen);
    XVisualInfo *visinfo = glXChooseVisual(context.MainDisplay, screen, attrib);

    if (!visinfo)
    {
        FatalError("Error: couldn't create OpenGL window with this pixel format.\n");
    }

    XSetWindowAttributes attr;
    attr.background_pixel = 0;
    attr.border_pixel = 0;
    attr.colormap = XCreateColormap(context.MainDisplay, root, visinfo->visual, AllocNone);
    attr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask | KeyReleaseMask;
    unsigned long mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

    context.MainWindow = XCreateWindow(
        context.MainDisplay,
        root,
        0, 0,
        ViewportWidth, ViewportHeight, 0,
        visinfo->depth,
        InputOutput,
        visinfo->visual,
        mask,
        &attr
    );

    XStoreName(context.MainDisplay, context.MainWindow, GetWindowName());

    GLXContext glcontext = glXCreateContext(context.MainDisplay, visinfo, NULL, True);
    glXMakeCurrent(context.MainDisplay, context.MainWindow, glcontext);
    XMapWindow(context.MainDisplay, context.MainWindow);

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        FatalError("GLEW Error: %s\n", glewGetErrorString(err));
    }

    DebugString("OpenGL Version: %s\n", glGetString(GL_VERSION));
    PrettyPrintExtensions();
    
    // -------------------
    // Start the Game Loop
    // -------------------

    Initialize();
    unsigned int previousTime = GetMilliseconds();

    bool done = false;
    while (!done)
    {
        if (glGetError() != GL_NO_ERROR)
        {
            FatalError("OpenGL error.\n");
        }

        if (XPending(context.MainDisplay))
        {
            XEvent event;
    
            XNextEvent(context.MainDisplay, &event);
            switch (event.type)
            {
                case Expose:
                    //redraw(display, event.xany.window);
                    break;
                
                case ConfigureNotify:
                    //resize(event.xconfigure.width, event.xconfigure.height);
                    break;
                
                case KeyRelease:
                case KeyPress:
                {
                    XComposeStatus composeStatus;
                    char asciiCode[32];
                    KeySym keySym;
                    int len;
                    
                    len = XLookupString(&event.xkey, asciiCode, sizeof(asciiCode), &keySym, &composeStatus);
                    switch (asciiCode[0])
                    {
                        case 'x': case 'X': case 'q': case 'Q':
                        case 0x1b:
                            done = true;
                            break;
                    }
                }
            }
        }

        unsigned int waitTime = 0;
        unsigned int currentTime = GetMilliseconds();
        unsigned int deltaTime = currentTime - previousTime;
        previousTime = currentTime;
        
        Update(deltaTime);

        Render();
        glXSwapBuffers(context.MainDisplay, context.MainWindow);
    }

    return 0;
}


#endif