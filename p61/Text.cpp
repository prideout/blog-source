#include <windows.h>
#include <gdiplus.h>
#include <string>
#include "Common.hpp"

#pragma comment(lib,"gdiplus.lib")

using namespace std;
using namespace Gdiplus;

struct OverlayContext
{
    string PreviousMessage;
    Bitmap* GdiBitmap;
    Texture MessageTexture;
};

static OverlayContext oc;

extern HDC DcBackbuffer;

Texture OverlayText(string message)
{
    PezConfig cfg = PezGetConfig();

    // Create a new text context if it doesn't already exist:
    static bool first = true;
    if (first) {
        first = false;

        GdiplusStartupInput gdiplusStartupInput;
        ULONG_PTR gdiplusToken;
        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, 0);

        oc.GdiBitmap = new Bitmap(cfg.Width, cfg.Height, PixelFormat32bppARGB);
        glGenTextures(1, &oc.MessageTexture.Handle);

        oc.MessageTexture.Width = cfg.Width;
        oc.MessageTexture.Height = cfg.Height;
    }

    // Skip GDI text generation if the string is unchanged:
    if (message == oc.PreviousMessage)
        return oc.MessageTexture;

    oc.PreviousMessage = message;

    // Create the GDI+ drawing context and set it up:
    Graphics* gfx = Graphics::FromImage(oc.GdiBitmap);
    gfx->Clear(Color::Transparent);
    gfx->SetSmoothingMode(SmoothingModeAntiAlias);
    gfx->SetInterpolationMode(InterpolationModeHighQualityBicubic);

    // Select a font:
    FontFamily fontFamily(L"Trebuchet MS");
    const float fontSize = 24;
    PointF origin(10.0f, 10.0f);
    StringFormat format(StringAlignmentNear);

    // Vertically flip it around for OpenGL:
    if (true) {
        Matrix flip;
        flip.Scale(1, -1);
        flip.Translate(0, -float(cfg.Height));
        gfx->SetTransform(&flip);
    }

    // Create a path along the outline of the glyphs:
    GraphicsPath path;
    path.AddString(
        wstring(message.begin(), message.end()).c_str(),
        -1,
        &fontFamily,
        FontStyleRegular,
        fontSize,
        origin,
        &format);

    // Draw some glow to steer clear of crappy AA:
    for (float width = 1; width < 4; ++width) {
        Pen pen(Color(64, 0, 0, 0), width);
        pen.SetLineJoin(LineJoinRound);
        gfx->DrawPath(&pen, &path);
    }

    // Outline the glyphs:
    Pen pen(Color(255, 0, 0, 0), 1);
    pen.SetLineJoin(LineJoinRound);
    gfx->DrawPath(&pen, &path);

    // Fill the glyphs:
    //SolidBrush brush(Color(0, 50, 100));
    SolidBrush brush(Color(255, 255, 255));
    gfx->FillPath(&brush, &path);

    // Lock the raw pixel data and pass it to OpenGL:
    BitmapData data;
    oc.GdiBitmap->LockBits(0, ImageLockModeRead, PixelFormat32bppARGB, &data);
    _ASSERT(data.Stride == sizeof(unsigned int) * cfg.Width);
    glBindTexture(GL_TEXTURE_2D, oc.MessageTexture.Handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, cfg.Width, cfg.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.Scan0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    oc.GdiBitmap->UnlockBits(&data);

    return oc.MessageTexture;
}

