#include "Splat.h"
#include <fstream>
#include <string>
#include <cmath>

using namespace std;

GLuint CreateSplat(GLuint quadVao, PointList positions)
{
    const int Size = 64;

    Surface surface = CreateVolume(Size, Size, Size);
    GLuint program = CreateProgram("Splat.VS", "Splat.GS", "Splat.FS");
    glUseProgram(program);

    GLint inverseSize = glGetUniformLocation(program, "InverseSize");
    glUniform1f(inverseSize, 1.0f / (float) Size);

    const float innerScale = 0.4f;

    GLint inverseVariance = glGetUniformLocation(program, "InverseVariance");
    glUniform1f(inverseVariance, -1.0f / (2.0f * innerScale * innerScale));

    GLint normalizationConstant = glGetUniformLocation(program, "NormalizationConstant");
    glUniform1f(normalizationConstant, 1.0f / std::pow(std::sqrt(TwoPi) * innerScale, 3.0f));

    glBindFramebuffer(GL_FRAMEBUFFER, surface.FboHandle);
    glBindTexture(GL_TEXTURE_3D, 0);
    glViewport(0, 0, Size, Size);
    glBindVertexArray(quadVao);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    PointList::const_iterator i = positions.begin();
    for (; i != positions.end(); ++i) {

        PointList::const_iterator next = i;
        if (++next == positions.end())
            next = positions.begin();
        VectorMath::Vector3 velocity = (*next - *i);

        GLint center = glGetUniformLocation(program, "Center");
        glUniform4f(center, i->getX(), i->getY(), i->getZ(), 0);

        GLint color = glGetUniformLocation(program, "Color");
        glUniform3fv(color, 1, (float*) &velocity);

        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, Size);
    }

    PezCheckCondition(GL_NO_ERROR == glGetError(), "Unable to create splat.");
    
    glViewport(0, 0, PEZ_VIEWPORT_WIDTH, PEZ_VIEWPORT_HEIGHT);
    glDisable(GL_BLEND);
    
    return surface.TextureHandle[0];
}

GLuint CreateCpuSplat(GLuint quadVao)
{
    const int Size = 64;
    const float InnerScale = 0.4f;
    const float RadiusScale = 0.001f;

    char* pixels = new char[Size*Size*Size];
    {
        // http://www.stat.wisc.edu/~mchung/teaching/MIA/reading/diffusion.gaussian.kernel.pdf.pdf
        float doubleVariance = 2.0f * InnerScale * InnerScale;
        float normalizationConstant = 1.0f / std::pow(std::sqrt(TwoPi) * InnerScale, 3.0f);
        char* pDest = pixels;
        float maxDensity = 0;
        float sumDensity = 0;
        float minDensity = 100.0f;
        for (int z = 0; z < Size; ++z) {
            for (int y = 0; y < Size; ++y) {
                for (int x = 0; x < Size; ++x) {
                    int cx = x - Size / 2;
                    int cy = y - Size / 2;
                    int cz = z - Size / 2;
                    float r2 = RadiusScale * float(cx*cx + cy*cy + cz*cz);
                    float density = normalizationConstant * std::exp(-r2 / doubleVariance);
                    maxDensity = std::max(maxDensity, density);
                    minDensity = std::min(minDensity, density);
                    sumDensity += density;
                    *pDest++ = (char) (255.0f * density);
                }
            }
        }
        PezDebugString("maximum = %f\n", maxDensity);
        PezDebugString("minimum = %f\n", minDensity);
        PezDebugString("sum = %f\n", sumDensity);
    }

    GLuint textureHandle;
    glGenTextures(1, &textureHandle);
    glBindTexture(GL_TEXTURE_3D, textureHandle);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, Size, Size, Size, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, pixels);

    delete [] pixels;

    return textureHandle;
}

GLuint CreateTeapot()
{
    int Width = 256;
    int Height = 256;
    int Depth = 178;

    char* pixels = new char[Width * Height * Depth];
    {
        // http://www.gris.uni-tuebingen.de/edu/areas/scivis/volren/datasets/datasets.html
        string path = string(PezResourcePath()) + string("/BostonTeapot-256x256x178.raw");
        ifstream f(path.c_str(), ios::in | ios::binary);
        PezCheckCondition((f.rdstate() & ifstream::failbit) == 0, "Failed to open voxel data.");
        f.seekg (0, ios::end);
        size_t length = (size_t) f.tellg();
        f.seekg (0, ios::beg);
        f.read(pixels, Width * Height * Depth);
    }

    GLuint textureHandle;
    glGenTextures(1, &textureHandle);
    glBindTexture(GL_TEXTURE_3D, textureHandle);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, Width, Height, Depth, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, pixels);

    delete [] pixels;

    return textureHandle;
}
