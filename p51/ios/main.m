#import <UIKit/UIKit.h>
#import <mach/mach_time.h>
#import <wchar.h>
#import <stdio.h>

#define countof(a) (sizeof(a)/sizeof(a[0]))

int main(int argc, char *argv[]) {
    
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    int retVal = UIApplicationMain(argc, argv, nil, nil);
    [pool release];
    return retVal;
}

const char* PezResourcePath()
{
    NSString* bundlePath =[[NSBundle mainBundle] resourcePath];
    return [bundlePath UTF8String];
}

void PezDebugStringW(const wchar_t* pStr, ...)
{
    va_list a;
    va_start(a, pStr);
	
    wchar_t msg[1024] = {0};
    vswprintf(msg, countof(msg), pStr, a);
    fputws(msg, stderr);
}

void PezDebugString(const char* pStr, ...)
{
    va_list a;
    va_start(a, pStr);
	
    char msg[1024] = {0};
    vsnprintf(msg, countof(msg), pStr, a);
    fputs(msg, stderr);
}

void _PezFatalErrorW(const wchar_t* pStr, va_list a)
{
    wchar_t msg[1024] = {0};
    vswprintf(msg, countof(msg), pStr, a);
    fputws(msg, stderr);
    exit(1);
}

void _PezFatalError(const char* pStr, va_list a)
{
    char msg[1024] = {0};
    vsnprintf(msg, countof(msg), pStr, a);
    puts(msg);
    puts("\n");
    exit(1);
}

void PezFatalError(const char* pStr, ...)
{
    va_list a;
    va_start(a, pStr);
    _PezFatalError(pStr, a);
}

void PezFatalErrorW(const wchar_t* pStr, ...)
{
    va_list a;
    va_start(a, pStr);
    _PezFatalErrorW(pStr, a);
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

void PezCheckConditionW(int condition, ...)
{
    va_list a;
    const wchar_t* pStr;
    
    if (condition)
        return;
    
    va_start(a, condition);
    pStr = va_arg(a, const wchar_t*);
    _PezFatalErrorW(pStr, a);
}
