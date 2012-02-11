#import <Cocoa/Cocoa.h>
#import <wchar.h>
#import "../tinylib/pez.h"

int main(int argc, char *argv[])
{
    return NSApplicationMain(argc,  (const char **) argv);
}

void PezErrorStringW(const wchar_t* pStr, ...)
{
    va_list a;
    va_start(a, pStr);

    wchar_t msg[1024] = {0};
    vswprintf(msg, countof(msg), pStr, a);
    fputws(msg, stderr);
}

void PezErrorString(const char* pStr, ...)
{
    va_list a;
    va_start(a, pStr);

    char msg[1024] = {0};
    vsnprintf(msg, countof(msg), pStr, a);
    fputs(msg, stderr);
}

void PezFatalErrorW(const wchar_t* pStr, ...)
{
    fwide(stderr, 1);

    va_list a;
    va_start(a, pStr);

    wchar_t msg[1024] = {0};
    vswprintf(msg, countof(msg), pStr, a);
    fputws(msg, stderr);
    exit(1);
}

void PezFatalError(const char* pStr, ...)
{
    va_list a;
    va_start(a, pStr);

    char msg[1024] = {0};
    vsnprintf(msg, countof(msg), pStr, a);
    fputs(msg, stderr);
    exit(1);
}

const char* PezGetDesktopFolder()
{
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDesktopDirectory, NSUserDomainMask, NO);
    return [[paths objectAtIndex:0] UTF8String];
}

const char* PezGetAssetsFolder()
{
    return [[[NSBundle mainBundle] resourcePath] UTF8String];
}

const char* PezOpenFileDialog()
{
    NSOpenPanel *op = [NSOpenPanel openPanel];
    if ([op runModal] == NSOKButton)
    {
        return [[op filename] UTF8String];
    }
    return 0;
}

void _PezFatalError(const char* pStr, va_list a)
{
    char msg[1024] = {0};
    vsnprintf(msg, countof(msg), pStr, a);
    fputs(msg, stderr);
    exit(1);
}

void PezCheckCondition(int condition, ...)
{
    va_list a;
    const char* pStr;

    if (condition)
        return;

    va_start(a, condition);
    pStr = va_arg(a, const char*);
    _PezFatalError(pStr, a);
}
