#include "../Common.hpp"
#import <QuartzCore/QuartzCore.h>
#include <string>

using std::string;

struct OverlayContext
{
    string PreviousMessage;
    unsigned char* ImageData;
    Texture MessageTexture;
    CGContextRef QuartzContext;
};

static OverlayContext oc;

Texture OverlayText(std::string message)
{
    PezConfig cfg = PezGetConfig();

    // Create a new text context if it doesn't already exist:
    static bool first = true;
    if (first) {
        first = false;

        oc.MessageTexture.Width = cfg.Width;
        oc.MessageTexture.Height = cfg.Height;
        
        const int BitsPerComponent = 8;
        oc.MessageTexture.Format = GL_RGBA;
    
        int bpp = BitsPerComponent / 2;
        int byteCount = cfg.Width * cfg.Height * bpp;
        oc.ImageData = (unsigned char*) calloc(byteCount, 1);
        
        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
        CGBitmapInfo bitmapInfo = kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big;
        oc.QuartzContext = CGBitmapContextCreate(oc.ImageData,
                                                     oc.MessageTexture.Width,
                                                     oc.MessageTexture.Height,
                                                     BitsPerComponent,
                                                     bpp * oc.MessageTexture.Width,
                                                     colorSpace,
                                                     bitmapInfo);
        CGColorSpaceRelease(colorSpace);
        CGContextSelectFont(oc.QuartzContext, "Monaco", 14, kCGEncodingMacRoman);
        CGContextSetTextDrawingMode(oc.QuartzContext, kCGTextFill);
        CGContextSetRGBFillColor(oc.QuartzContext, 1.0, 1.0, 1.0, 1.0);
        CGAffineTransform xform = CGAffineTransformMake(1.0, 0.0, 0.0, -1.0, 0.0, 0.0);
        CGContextSetTextMatrix(oc.QuartzContext, xform);

        glGenTextures(1, &oc.MessageTexture.Handle);
    }

    // Skip GDI text generation if the string is unchanged:
    if (message == oc.PreviousMessage)
        return oc.MessageTexture;

    oc.PreviousMessage = message;

    float x = 16;
    float y = 16;
    CGContextClearRect(oc.QuartzContext, CGRectMake(0, 0, cfg.Width, cfg.Height));
    CGContextShowTextAtPoint(oc.QuartzContext, x, y, message.c_str(), message.size());

    glBindTexture(GL_TEXTURE_2D, oc.MessageTexture.Handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, cfg.Width, cfg.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, oc.ImageData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return oc.MessageTexture;
}
