#include "Splat.h"

GLuint CreateNoise()
{
    int Width = 256;
    int Height = 256;

    char* pixels = new char[Width * Height];
    
    char* pDest = pixels;
    for (int i = 0; i < Width * Height; i++) {
        *pDest++ = rand() % 256;
    }

    GLuint textureHandle;
    glGenTextures(1, &textureHandle);
    glBindTexture(GL_TEXTURE_2D, textureHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, Width, Height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, pixels);

    delete [] pixels;

    return textureHandle;
}
