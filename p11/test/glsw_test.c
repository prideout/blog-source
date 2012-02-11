#include <stdio.h>
#include "glsw.h"

int bEnableOutput = 1;

//#define MEMORY_LEAK_TEST_1
//#define MEMORY_LEAK_TEST_2

void test(const char* shaderKey)
{
    const char* shaderSource = glswGetShader(shaderKey);

    if (!bEnableOutput)
        return;

    if (!shaderSource)
    {
        printf("Error with %s: %s\n\n\n", shaderKey, glswGetError());
    }
    else
    {
        printf("------------------%s:\n%s\n\n\n", shaderKey, shaderSource);
    }
}

// TODO
void randomtest()
{
    // 1. Create temporary file and populate with a couple boring shaders
    // 2. Get a string from it. (success expected)
    // 3. Get a string from it. (fail expected)
}

void partialtest()
{
    test("Blit.Vertex");
    test("Blit.Fragment");

    test("SignedEuclidean.Vertex.GL3.Erosion");
    test("SignedEuclidean.Vertex.GL3.Erosion.ModeDoesNotExist");
    test("SignedEuclidean.Vertex.ApiDoesNotExist");
    test("SignedEuclidean.StageDoesNotExist");
    test("EffectDoesNotExist");
    test("SignedEuclidean.Fragment.GL3.Erosion.Cilantro");
    test("SignedEuclidean.Fragment.GL3.Erosion.HorizontalRed");
    test("SignedEuclidean.Fragment.GL3.Erosion.HorizontalGreen");
    test("SignedEuclidean.Fragment.GL3.Erosion.VerticalRed");
    test("SignedEuclidean.Fragment.GL3.Erosion.VerticalGreen");
    
    // TODO we should create one other effect file

    test("SignedEuclidean.Vertex.GL2.Blit");
}

void starttest()
{
    glswInit();

#ifdef __MAC_NA
    glswSetPath("../../test/", ".glsl");
#else
    glswSetPath("../test/", ".glsl");
#endif

    glswAddDirectiveToken("GL2", "#version 120");
    glswAddDirectiveToken("GL3", "#version 130");
    glswAddDirectiveToken("GL4", "#version 150");
}

void fulltest()
{
    starttest();
    partialtest();
    randomtest();
    glswShutdown();

    {
        const char* shaderSource = glswGetShader("SignedEuclidean.Vertex.GL3.Erosion");
        if (bEnableOutput)
        {
            if (shaderSource)
                printf("Bad; should've reported an error!");
            else
                printf("Good, got another expected error: : %s\n\n\n", glswGetError());
        }
    }
}

int main()
{
    fulltest();

#ifdef MEMORY_LEAK_TEST_1
    bEnableOutput = 0;
    while (1) { fulltest(); }
#endif

#ifdef MEMORY_LEAK_TEST_2
    // Memory leak test 2
    bEnableOutput = 0;
    starttest();
    while (1) { partialtest(); }
#endif

    return 0;
}
