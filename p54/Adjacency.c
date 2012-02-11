#include "Platform.h"
#include <Judy.h>

// We use a Judy array for vertex-to-edge mapping.
// See http://judy.sourceforge.net/ for more on Judy arrays.

#define JUDY_VAL_TYPE              HalfEdge*
#define JUDY_ADD(TABLE, KEY, VAL)  *((JUDY_VAL_TYPE*) JudyLIns(&TABLE, KEY, PJE0)) = VAL
#define JUDY_FIRST(TABLE, KEY)     (JUDY_VAL_TYPE*) JudyLFirst(TABLE, &KEY, PJE0)
#define JUDY_NEXT(TABLE, KEY)      (JUDY_VAL_TYPE*) JudyLNext(TABLE, &KEY, PJE0)
#define JUDY_GET(TABLE, KEY)       (JUDY_VAL_TYPE*) JudyLGet(TABLE, KEY, PJE0)
#define JUDY_COUNT(TABLE)          JudyLCount(TABLE, 0, -1, PJE0);
#define JUDY_FREE(TABLE)           JudyLFreeArray(&TABLE, PJE0);

typedef struct HalfEdgeRec
{
    unsigned short Vert;      // Vertex index at the end of this half-edge
    struct HalfEdgeRec* Twin; // Oppositely oriented adjacent half-edge
    struct HalfEdgeRec* Next; // Next half-edge around the face
} HalfEdge;

void ComputeAdjacency(unsigned short* dest, const unsigned short* source, int faceCount, int vertCount)
{
    // Allocate all pieces of the half-edge data structure:
    HalfEdge* edges = (HalfEdge*) calloc(faceCount * 3, sizeof(HalfEdge));

    // Declare a Judy array to help build the half-edge structure:
    //   - Keys are pairs of vertex indices
    //   - Values are pointers to half-edges
    void* edgeTable = 0;

    // Plow through faces and fill all half-edge info except twin pointers:
    const unsigned short* pSrc = source;
    HalfEdge* pEdge = edges;
    for (int faceIndex = 0; faceIndex < faceCount; ++faceIndex)
    {
        unsigned short A = *pSrc++;
        unsigned short B = *pSrc++;
        unsigned short C = *pSrc++;

        // Create the half-edge that goes from C to A:
        JUDY_ADD(edgeTable, C | (A << 16), pEdge);
        pEdge->Vert = A;
        pEdge->Next = 1 + pEdge;
        ++pEdge;

        // Create the half-edge that goes from A to B:
        JUDY_ADD(edgeTable, A | (B << 16), pEdge);
        pEdge->Vert = B;
        pEdge->Next = 1 + pEdge;
        ++pEdge;

        // Create the half-edge that goes from B to C:
        JUDY_ADD(edgeTable, B | (C << 16), pEdge);
        pEdge->Vert = C;
        pEdge->Next = pEdge - 2;
        ++pEdge;
    }

    // Verify that the mesh is clean:
    unsigned long numEntries = JUDY_COUNT(edgeTable);
    PezCheckCondition(numEntries == faceCount * 3, "Bad mesh: duplicated edges or inconsistent winding.");

    // Populate the twin pointers by iterating over the Judy array:
    unsigned long edgeIndex = 0; 
    HalfEdge** ppEdge = JUDY_FIRST(edgeTable, edgeIndex);
    int boundaryCount = 0;
    while (ppEdge)
    {
        unsigned long twinIndex = ((edgeIndex & 0xffff) << 16) | (edgeIndex >> 16);
        HalfEdge** ppTwinEdge = JUDY_GET(edgeTable, twinIndex);
        if (ppTwinEdge)
        {
            (*ppTwinEdge)->Twin = *ppEdge;
            (*ppEdge)->Twin = *ppTwinEdge;
        }
        else
        {
            ++boundaryCount;
        }
        ppEdge = JUDY_NEXT(edgeTable, edgeIndex);
    }

    // Now that we have a half-edge structure, it's easy to create adjacency info for OpenGL:
    if (boundaryCount > 0)
    {
        PezDebugString("Mesh is not watertight.  Contains %d boundary edges.\n", boundaryCount);

        unsigned short* pDest = dest;
        HalfEdge* pEdge = edges;
        for (int faceIndex = 0; faceIndex < faceCount; ++faceIndex, pEdge += 3, pDest += 6)
        {
            pDest[0] = pEdge[2].Vert;
            pDest[1] = pEdge[0].Twin ? (pEdge[0].Twin->Next->Vert) : pDest[0];
            pDest[2] = pEdge[0].Vert;
            pDest[3] = pEdge[1].Twin ? (pEdge[1].Twin->Next->Vert) : pDest[1];
            pDest[4] = pEdge[1].Vert;
            pDest[5] = pEdge[2].Twin ? (pEdge[2].Twin->Next->Vert) : pDest[2];
        }
    }
    else
    {
        unsigned short* pDest = dest;
        HalfEdge* pEdge = edges;
        for (int faceIndex = 0; faceIndex < faceCount; ++faceIndex, pEdge += 3, pDest += 6)
        {
            pDest[0] = pEdge[2].Vert;
            pDest[1] = pEdge[0].Twin->Next->Vert;
            pDest[2] = pEdge[0].Vert;
            pDest[3] = pEdge[1].Twin->Next->Vert;
            pDest[4] = pEdge[1].Vert;
            pDest[5] = pEdge[2].Twin->Next->Vert;
        }
    }

    JUDY_FREE(edgeTable);
    free(edges);
}
