#include <windows.h>
#include <mmsystem.h>
#include <d3dx9.h>
#include <openctm.h>
#include <string>
#include <set>
#include <iostream>

using namespace std;

const bool CheckForDuplicates = false;
const bool RemoveNormals = true;
const bool RemoveTexCoords = true;
const bool WeldVertices = true;

LPDIRECT3D9 g_pD3D = 0;
LPDIRECT3DDEVICE9 g_pd3dDevice = 0;
LPD3DXMESH g_pMesh = 0;
D3DMATERIAL9* g_pMeshMaterials = 0;
LPDIRECT3DTEXTURE9* g_pMeshTextures = 0;
DWORD g_dwNumMaterials = 0;

void InitializeD3D(HWND hWnd);
void CreateGeometry(const char* sourceFile);
void Render();
void Cleanup();

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void ExportRangeCTM(D3DXATTRIBUTERANGE range, ID3DXMesh* pMesh, const char* destFile)
{
    // Find where the positions, normals, and texture coordinates live.

    const WORD MISSING_ATTRIBUTE = 0xffff;
    WORD positionsOffset = MISSING_ATTRIBUTE;
    WORD normalsOffset = MISSING_ATTRIBUTE;
    WORD texCoordsOffset = MISSING_ATTRIBUTE;

    D3DVERTEXELEMENT9 vertexLayout[MAX_FVF_DECL_SIZE];
    D3DVERTEXELEMENT9 endMarker = D3DDECL_END();
    pMesh->GetDeclaration(vertexLayout);
    D3DVERTEXELEMENT9* pVertexLayout = &vertexLayout[0];
    for (int attrib = 0;  attrib < MAX_FVF_DECL_SIZE; ++attrib, pVertexLayout++)
    {
        if (0 == memcmp(&vertexLayout[attrib], &endMarker, sizeof(endMarker)))
        {
            break;
        }
        if (pVertexLayout->Stream != 0)
        {
            cout << "Nonzero stream: " << pVertexLayout->Stream << endl;
            continue;
        }
        if (pVertexLayout->Usage == D3DDECLUSAGE_POSITION && pVertexLayout->Type == D3DDECLTYPE_FLOAT3)
        {
            cout << "Contains positions " << (int) pVertexLayout->UsageIndex << endl;
            positionsOffset = pVertexLayout->Offset;
        }
        else if (pVertexLayout->Usage == D3DDECLUSAGE_NORMAL && pVertexLayout->Type == D3DDECLTYPE_FLOAT3)
        {
            cout << "Contains normals " << (int) pVertexLayout->UsageIndex << endl;
            normalsOffset = pVertexLayout->Offset;
            if (RemoveNormals)
                cout << "I will strip normals.\n";
            else
                cout << "I will preserve normals.\n";
        }
        else if (pVertexLayout->Usage == D3DDECLUSAGE_TEXCOORD && pVertexLayout->Type == D3DDECLTYPE_FLOAT2)
        {
            cout << "Contains texture coordinates " << (int) pVertexLayout->UsageIndex << endl;
            texCoordsOffset = pVertexLayout->Offset;
            if (RemoveTexCoords)
                cout << "I will strip texture coordinates.\n";
            else
                cout << "I will preserve texture coordinates.\n";
        }
        else
        {
            cout << "Mysterious attribute" << endl;
        }
    }

    // Check that we support the format of the data.

    if (positionsOffset == MISSING_ATTRIBUTE)
    {
        cerr << "Unsupported Vertex Declaration in " << destFile << endl;
        exit(1);
    }

    // Obtain vertex & index counts from the D3D mesh; allocate memory for the CTM mesh.

    DWORD dwVertexCount = pMesh->GetNumVertices();
    DWORD dwTriangleCount = pMesh->GetNumFaces();

//dwVertexCount = range.VertexCount;
    dwTriangleCount = range.FaceCount;

    DWORD dwIndexCount = dwTriangleCount * 3;
    CTMfloat* pCtmVertices = new CTMfloat[3* dwVertexCount];
    CTMuint* pCtmIndices = new CTMuint[dwIndexCount];
    CTMfloat* pCtmNormals = (normalsOffset == MISSING_ATTRIBUTE) ? 0 : new CTMfloat[3*dwVertexCount];
    CTMfloat* pCtmTexCoords = (texCoordsOffset == MISSING_ATTRIBUTE) ? 0 : new CTMfloat[2*dwVertexCount];

    if (RemoveNormals)
    {
        delete pCtmNormals;
        pCtmNormals = 0;
        normalsOffset = MISSING_ATTRIBUTE;
    }

    if (RemoveTexCoords)
    {
        delete pCtmTexCoords;
        pCtmTexCoords = 0;
        texCoordsOffset = MISSING_ATTRIBUTE;
    }

//positionsOffset += range.VertexStart * pMesh->GetNumBytesPerVertex();
//if (normalsOffset != MISSING_ATTRIBUTE) normalsOffset += range.VertexStart * pMesh->GetNumBytesPerVertex();
//if (texCoordsOffset != MISSING_ATTRIBUTE) texCoordsOffset += range.VertexStart * pMesh->GetNumBytesPerVertex();

    // Lock down the verts and pluck out the positions and normals.
    {
        void* pData = 0;
        pMesh->LockVertexBuffer(D3DLOCK_READONLY , &pData);

        if (positionsOffset != MISSING_ATTRIBUTE)
        {
            unsigned char* pSource = ((unsigned char*) pData) + positionsOffset;
            unsigned char* pDest = (unsigned char* ) pCtmVertices;
            DWORD dwSourceStride = pMesh->GetNumBytesPerVertex();
            DWORD dwDestStride = sizeof(CTMfloat) * 3;

            for (DWORD dwVertex = 0; dwVertex < dwVertexCount; ++dwVertex)
            {
                memcpy(pDest, pSource, dwDestStride);
                pSource += dwSourceStride;
                pDest += dwDestStride;
            }
        }

        if (normalsOffset != MISSING_ATTRIBUTE)
        {
            unsigned char* pSource = ((unsigned char*) pData) + normalsOffset;
            unsigned char* pDest = (unsigned char* ) pCtmNormals;
            DWORD dwSourceStride = pMesh->GetNumBytesPerVertex();
            DWORD dwDestStride = sizeof(CTMfloat) * 3;

            for (DWORD dwVertex = 0; dwVertex < dwVertexCount; ++dwVertex)
            {
                memcpy(pDest, pSource, dwDestStride);
                pSource += dwSourceStride;
                pDest += dwDestStride;
            }
        }

        if (texCoordsOffset != MISSING_ATTRIBUTE)
        {
            unsigned char* pSource = ((unsigned char*) pData) + texCoordsOffset;
            unsigned char* pDest = (unsigned char* ) pCtmTexCoords;
            DWORD dwSourceStride = pMesh->GetNumBytesPerVertex();
            DWORD dwDestStride = sizeof(CTMfloat) * 2;

            for (DWORD dwVertex = 0; dwVertex < dwVertexCount; ++dwVertex)
            {
                memcpy(pDest, pSource, dwDestStride);
                pSource += dwSourceStride;
                pDest += dwDestStride;
            }
        }

        pMesh->UnlockVertexBuffer();
    }

    // Lock down the indices and convert them to unsigned 32-bit integers.
    {
        void* pData = 0;
        pMesh->LockIndexBuffer(D3DLOCK_READONLY , &pData);

        DWORD dwOptions = pMesh->GetOptions();
        DWORD dwSourceStride = (dwOptions & D3DXMESH_32BIT) ? 4 : 2;

        unsigned char* pSource = (unsigned char*) pData;
        unsigned char* pDest = (unsigned char*) pCtmIndices;

        pSource += range.FaceStart * (dwSourceStride * 3);

        DWORD dwDestStride = sizeof(CTMuint);

        bool duplicates = false;
        std::set<int> inds;

        for (DWORD dwIndex = 0; dwIndex < dwIndexCount; ++dwIndex)
        {
            memset(pDest, 0, dwDestStride);
            memcpy(pDest, pSource, dwSourceStride);

//int* pIntDest= (int*) pDest;
//*(pIntDest) -= range.VertexStart;

            if (CheckForDuplicates)
            {
                int* ind = (int*) pDest;
                if (!duplicates && inds.find(*ind) != inds.end())
                {
                    duplicates = true;
                }
                else
                {
                    inds.insert(*ind);
                }
            }

            pSource += dwSourceStride;
            pDest += dwDestStride;
        }

        if (CheckForDuplicates && !duplicates)
        {
            cerr << "No duplicated indices -- you need weld your verts!" << endl;
            exit(1);
        }

        pMesh->UnlockIndexBuffer();
    }

    // Instance the OpenCTM mesh and save it to disk.
    CTMcontext context = ctmNewContext(CTM_EXPORT);
    ctmDefineMesh(context, pCtmVertices, dwVertexCount, pCtmIndices, dwTriangleCount, pCtmNormals);
    if (pCtmTexCoords)
    {
        cout << "Exporting texcoords...\n";
        ctmAddUVMap(context, pCtmTexCoords, "TexCoords", NULL);
    }
    ctmSave(context, destFile);
    ctmFreeContext(context);

    // Free the OpenCTM buffers.
    delete [] pCtmVertices;
    delete [] pCtmIndices;
    delete [] pCtmNormals;
    delete [] pCtmTexCoords;
}

void ExportCTM(ID3DXMesh* pMesh, const char* destFile)
{
    // Find where the positions, normals, and texture coordinates live.

    const WORD MISSING_ATTRIBUTE = 0xffff;
    WORD positionsOffset = MISSING_ATTRIBUTE;
    WORD normalsOffset = MISSING_ATTRIBUTE;
    WORD texCoordsOffset = MISSING_ATTRIBUTE;

    D3DVERTEXELEMENT9 vertexLayout[MAX_FVF_DECL_SIZE];
    D3DVERTEXELEMENT9 endMarker = D3DDECL_END();
    pMesh->GetDeclaration(vertexLayout);
    D3DVERTEXELEMENT9* pVertexLayout = &vertexLayout[0];
    for (int attrib = 0;  attrib < MAX_FVF_DECL_SIZE; ++attrib, pVertexLayout++)
    {
        if (0 == memcmp(&vertexLayout[attrib], &endMarker, sizeof(endMarker)))
        {
            break;
        }
        if (pVertexLayout->Stream != 0)
        {
            cout << "Nonzero stream: " << pVertexLayout->Stream << endl;
            continue;
        }
        if (pVertexLayout->Usage == D3DDECLUSAGE_POSITION && pVertexLayout->Type == D3DDECLTYPE_FLOAT3)
        {
            cout << "Contains positions " << (int) pVertexLayout->UsageIndex << endl;
            positionsOffset = pVertexLayout->Offset;
        }
        else if (pVertexLayout->Usage == D3DDECLUSAGE_NORMAL && pVertexLayout->Type == D3DDECLTYPE_FLOAT3)
        {
            cout << "Contains normals " << (int) pVertexLayout->UsageIndex << endl;
            normalsOffset = pVertexLayout->Offset;
            if (RemoveNormals)
                cout << "I will strip normals.\n";
            else
                cout << "I will preserve normals.\n";
        }
        else if (pVertexLayout->Usage == D3DDECLUSAGE_TEXCOORD && pVertexLayout->Type == D3DDECLTYPE_FLOAT2)
        {
            cout << "Contains texture coordinates " << (int) pVertexLayout->UsageIndex << endl;
            texCoordsOffset = pVertexLayout->Offset;
            if (RemoveTexCoords)
                cout << "I will strip texture coordinates.\n";
            else
                cout << "I will preserve texture coordinates.\n";
        }
        else
        {
            cout << "Mysterious attribute" << endl;
        }
    }

    // Check that we support the format of the data.

    if (positionsOffset == MISSING_ATTRIBUTE)
    {
        cerr << "Unsupported Vertex Declaration in " << destFile << endl;
        exit(1);
    }

    // Obtain vertex & index counts from the D3D mesh; allocate memory for the CTM mesh.
    DWORD dwVertexCount = pMesh->GetNumVertices();
    DWORD dwTriangleCount = pMesh->GetNumFaces();
    DWORD dwIndexCount = dwTriangleCount * 3;
    CTMfloat* pCtmVertices = new CTMfloat[3* dwVertexCount];
    CTMuint* pCtmIndices = new CTMuint[dwIndexCount];
    CTMfloat* pCtmNormals = (normalsOffset == MISSING_ATTRIBUTE) ? 0 : new CTMfloat[3*dwVertexCount];
    CTMfloat* pCtmTexCoords = (texCoordsOffset == MISSING_ATTRIBUTE) ? 0 : new CTMfloat[2*dwVertexCount];

    if (RemoveNormals)
    {
        delete pCtmNormals;
        pCtmNormals = 0;
        normalsOffset = MISSING_ATTRIBUTE;
    }

    if (RemoveTexCoords)
    {
        delete pCtmTexCoords;
        pCtmTexCoords = 0;
        texCoordsOffset = MISSING_ATTRIBUTE;
    }

    // Lock down the verts and pluck out the positions and normals.
    {
        void* pData = 0;
        pMesh->LockVertexBuffer(D3DLOCK_READONLY , &pData);

        if (positionsOffset != MISSING_ATTRIBUTE)
        {
            unsigned char* pSource = ((unsigned char*) pData) + positionsOffset;
            unsigned char* pDest = (unsigned char* ) pCtmVertices;
            DWORD dwSourceStride = pMesh->GetNumBytesPerVertex();
            DWORD dwDestStride = sizeof(CTMfloat) * 3;
    
            for (DWORD dwVertex = 0; dwVertex < dwVertexCount; ++dwVertex)
            {
                memcpy(pDest, pSource, dwDestStride);
                pSource += dwSourceStride;
                pDest += dwDestStride;
            }
        }

        if (normalsOffset != MISSING_ATTRIBUTE)
        {
            unsigned char* pSource = ((unsigned char*) pData) + normalsOffset;
            unsigned char* pDest = (unsigned char* ) pCtmNormals;
            DWORD dwSourceStride = pMesh->GetNumBytesPerVertex();
            DWORD dwDestStride = sizeof(CTMfloat) * 3;
    
            for (DWORD dwVertex = 0; dwVertex < dwVertexCount; ++dwVertex)
            {
                memcpy(pDest, pSource, dwDestStride);
                pSource += dwSourceStride;
                pDest += dwDestStride;
            }
        }

        if (texCoordsOffset != MISSING_ATTRIBUTE)
        {
            unsigned char* pSource = ((unsigned char*) pData) + texCoordsOffset;
            unsigned char* pDest = (unsigned char* ) pCtmTexCoords;
            DWORD dwSourceStride = pMesh->GetNumBytesPerVertex();
            DWORD dwDestStride = sizeof(CTMfloat) * 2;
    
            for (DWORD dwVertex = 0; dwVertex < dwVertexCount; ++dwVertex)
            {
                memcpy(pDest, pSource, dwDestStride);
                pSource += dwSourceStride;
                pDest += dwDestStride;
            }
        }

        pMesh->UnlockVertexBuffer();
    }

    // Lock down the indices and convert them to unsigned 32-bit integers.
    {
        void* pData = 0;
        pMesh->LockIndexBuffer(D3DLOCK_READONLY , &pData);

        DWORD dwOptions = pMesh->GetOptions();
        DWORD dwSourceStride = (dwOptions & D3DXMESH_32BIT) ? 4 : 2;

        unsigned char* pSource = (unsigned char*) pData;
        unsigned char* pDest = (unsigned char*) pCtmIndices;
        
        DWORD dwDestStride = sizeof(CTMuint);

        bool duplicates = false;
        std::set<int> inds;

        for (DWORD dwIndex = 0; dwIndex < dwIndexCount; ++dwIndex)
        {
            memset(pDest, 0, dwDestStride);
            memcpy(pDest, pSource, dwSourceStride);
            
            if (CheckForDuplicates)
            {
                int* ind = (int*) pDest;
                if (!duplicates && inds.find(*ind) != inds.end())
                {
                    duplicates = true;
                }
                else
                {
                    inds.insert(*ind);
                }
            }

            pSource += dwSourceStride;
            pDest += dwDestStride;
        }

        if (CheckForDuplicates && !duplicates)
        {
            cerr << "No duplicated indices -- you need weld your verts!" << endl;
            exit(1);
        }

        pMesh->UnlockIndexBuffer();
    }

    // Instance the OpenCTM mesh and save it to disk.
    CTMcontext context = ctmNewContext(CTM_EXPORT);
    ctmDefineMesh(context, pCtmVertices, dwVertexCount, pCtmIndices, dwTriangleCount, pCtmNormals);
    if (pCtmTexCoords)
    {
        cout << "Exporting texcoords...\n";
        ctmAddUVMap(context, pCtmTexCoords, "TexCoords", NULL);
    }
    ctmSave(context, destFile);
    ctmFreeContext(context);

    // Free the OpenCTM buffers.
    delete [] pCtmVertices;
    delete [] pCtmIndices;
    delete [] pCtmNormals;
    delete [] pCtmTexCoords;
}

void GameLoop()
{
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, 0, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            Render();
        }
    }
}

int main(int argc, const char** argv)
{
    // Register the window class
    WNDCLASSEX wc =
    {
        sizeof(WNDCLASSEX),CS_CLASSDC, MsgProc, 0, 0,
        GetModuleHandle(0), 0, 0, 0, 0,
        L"X2CTM", 0
    };
    RegisterClassEx(&wc);

    HWND hWnd = CreateWindow(L"X2CTM", L"X2CTM",
                             WS_OVERLAPPEDWINDOW, 100, 100, 300, 300,
                             0, 0, wc.hInstance, 0 );

    if (argc < 2)
    {
        cout << "Usage: x2ctm [sourcefile] [sourcefile] ... " << endl;
    }

    for (int iarg = 1; iarg < argc; ++iarg)
    {
        InitializeD3D(hWnd);
        CreateGeometry(*++argv);
        ShowWindow(hWnd, SW_SHOWDEFAULT);
        UpdateWindow(hWnd);
        Render();
        // GameLoop();
        Cleanup();
    }

    UnregisterClass( L"X2CTM", wc.hInstance );
    return 0;
}

void InitializeD3D(HWND hWnd)
{
    g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

    g_pD3D->CreateDevice(
        D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL,
        hWnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING,
        &d3dpp,
        &g_pd3dDevice);

    g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
    g_pd3dDevice->SetRenderState(D3DRS_AMBIENT, 0xffffffff);
}

void CreateGeometry(const char* sourceFile)
{
    cout << endl << "Reading " << sourceFile << endl;
    wstring wideSourceFile(sourceFile, sourceFile + strlen(sourceFile));

    // Load the mesh from the specified file
    LPD3DXBUFFER pD3DXMtrlBuffer;
    LPD3DXBUFFER pD3DXEffectInstances;
    HRESULT hr = D3DXLoadMeshFromX(
        wideSourceFile.c_str(),
        D3DXMESH_SYSTEMMEM,
        g_pd3dDevice, 0,
        &pD3DXMtrlBuffer, &pD3DXEffectInstances, &g_dwNumMaterials,
        &g_pMesh);

    if (FAILED(hr))
    {
        MessageBox(0, (L"Could not find " + wideSourceFile).c_str(), L"X2CTM", MB_OK);
        exit(1);
    }

    DWORD* adjacencyIn = new DWORD[3 * g_pMesh->GetNumFaces()];
    g_pMesh->GenerateAdjacency(0.0001f, adjacencyIn);

    DWORD* adjacencyOut = new DWORD[3 * g_pMesh->GetNumFaces()];

    LPD3DXMESH newMesh = 0;
    //hr = g_pMesh->Optimize(D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_COMPACT, adjacencyIn, adjacencyOut, 0, 0, &newMesh);
    //hr = g_pMesh->OptimizeInplace(D3DXMESHOPT_ATTRSORT, adjacencyIn, adjacencyOut, 0, 0);
    if (FAILED(hr))
    {
        MessageBox(0, L"Unable to build attribute table", L"Whatever", MB_OK);
        exit(1);
    }
    //g_pMesh = newMesh;

    if (WeldVertices)
    {
        DWORD beforeVertCount = g_pMesh->GetNumVertices();
        DWORD beforeFaceCount = g_pMesh->GetNumFaces();
        hr = D3DXWeldVertices(g_pMesh, D3DXWELDEPSILONS_WELDALL, 0, 0, 0, 0, 0);
        DWORD afterVertCount = g_pMesh->GetNumVertices();
        DWORD afterFaceCount = g_pMesh->GetNumFaces();
    }

    D3DXATTRIBUTERANGE table[256];
    DWORD tableSize = sizeof(table) / sizeof(table[0]);
    g_pMesh->GetAttributeTable(&table[0], &tableSize);

    D3DXMATERIAL* d3dxMaterials = (D3DXMATERIAL*) pD3DXMtrlBuffer->GetBufferPointer();
    D3DXEFFECTINSTANCE* d3dxEffects = (D3DXEFFECTINSTANCE*) pD3DXEffectInstances->GetBufferPointer();

    g_pMeshMaterials = new D3DMATERIAL9[g_dwNumMaterials];
    g_pMeshTextures = new LPDIRECT3DTEXTURE9[g_dwNumMaterials];

    for (DWORD i = 0; i < g_dwNumMaterials; i++)
    {
        g_pMeshMaterials[i] = d3dxMaterials[i].MatD3D;
        g_pMeshMaterials[i].Ambient = g_pMeshMaterials[i].Diffuse;
        g_pMeshTextures[i] = 0;
        if (d3dxMaterials[i].pTextureFilename && lstrlenA(d3dxMaterials[i].pTextureFilename) > 0)
        {
            D3DXCreateTextureFromFileA(g_pd3dDevice, d3dxMaterials[i].pTextureFilename, &g_pMeshTextures[i]);
        }
    }
/*
    for (DWORD attrib = 0;  attrib < tableSize; ++attrib)
    {

        // I'm not so sure about material-to-attribute correlation
//        if (attrib < g_dwNumMaterials)
//        {
            LPSTR pTexture = d3dxMaterials[attrib].pTextureFilename;
            LPSTR pSlash = strchr(pTexture, '\\');
            if (pSlash)
            {
                pTexture = ++pSlash;
            }
            cout << "{Texture='" << pTexture << "',";
//        }

        string subMeshFilename = string("X_") + string("Armature.ctm"); // string(pTexture).substr(0, strlen(pTexture) - 4) + ".ctm";
        subMeshFilename[0] = attrib + 'A';

        ExportRangeCTM(table[attrib], g_pMesh, subMeshFilename.c_str());

        cout
            //<< table[attrib].AttribId << ' '
            << "FaceStart=" << table[attrib].FaceStart << ','
            << "FaceCount=" << table[attrib].FaceCount << ','
            << "VertexStart=" << table[attrib].VertexStart << ','
            << "VertexCount=" << table[attrib].VertexCount << '}' << endl;
    }
*/
    pD3DXMtrlBuffer->Release();


    // Convert the filename from .X to .CTM while preserving the full path.

   char destFile[_MAX_PATH];
   char drive[_MAX_DRIVE];
   char dir[_MAX_DIR];
   char fname[_MAX_FNAME];
   char ext[_MAX_EXT];
    
   _splitpath_s(sourceFile, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
   _makepath_s(destFile, _MAX_PATH, drive, dir, fname, "ctm");

    ExportCTM(g_pMesh, destFile);
    cout << "Exported " << destFile << endl;
/*
    const WORD MISSING_ATTRIBUTE = 0xffff;
    WORD positionsOffset = MISSING_ATTRIBUTE;
    WORD normalsOffset = MISSING_ATTRIBUTE;
    WORD texCoordsOffset = MISSING_ATTRIBUTE;

    D3DVERTEXELEMENT9 vertexLayout[MAX_FVF_DECL_SIZE];
    D3DVERTEXELEMENT9 endMarker = D3DDECL_END();
    g_pMesh->GetDeclaration(vertexLayout);
    D3DVERTEXELEMENT9* pVertexLayout = &vertexLayout[0];
    for (int attrib = 0;  attrib < MAX_FVF_DECL_SIZE; ++attrib, pVertexLayout++)
    {
        if (0 == memcmp(&vertexLayout[attrib], &endMarker, sizeof(endMarker)))
        {
            break;
        }
        if (pVertexLayout->Stream != 0)
        {
            cout << "Nonzero stream: " << pVertexLayout->Stream << endl;
            continue;
        }
        if (pVertexLayout->Usage == D3DDECLUSAGE_POSITION && pVertexLayout->Type == D3DDECLTYPE_FLOAT3)
        {
            cout << "Contains positions " << (int) pVertexLayout->UsageIndex << endl;
            positionsOffset = pVertexLayout->Offset;
        }
        else if (pVertexLayout->Usage == D3DDECLUSAGE_NORMAL && pVertexLayout->Type == D3DDECLTYPE_FLOAT3)
        {
            cout << "Contains normals " << (int) pVertexLayout->UsageIndex << endl;
            normalsOffset = pVertexLayout->Offset;
        }
        else if (pVertexLayout->Usage == D3DDECLUSAGE_TEXCOORD && pVertexLayout->Type == D3DDECLTYPE_FLOAT2)
        {
            cout << "Contains texture coordinates " << (int) pVertexLayout->UsageIndex << endl;
            texCoordsOffset = pVertexLayout->Offset;
        }
        else
        {
            cout << "Mysterious attribute" << endl;
        }
    }

    // Check that we support the format of the data.

    if (positionsOffset == MISSING_ATTRIBUTE)
    {
        exit(1);
    }

    // Obtain vertex & index counts from the D3D mesh; allocate memory for the CTM mesh.
    DWORD dwVertexCount = g_pMesh->GetNumVertices();
    DWORD dwTriangleCount = g_pMesh->GetNumFaces();
    DWORD dwIndexCount = dwTriangleCount * 3;

    // Lock down the verts and pluck out the positions and normals.
    {
        void* pData = 0;
        if (S_OK != g_pMesh->LockVertexBuffer(0, &pData))
        {
            exit(1);
        }

        if (positionsOffset != MISSING_ATTRIBUTE)
        {
            unsigned char* pSource = ((unsigned char*) pData) + positionsOffset;
            DWORD dwSourceStride = g_pMesh->GetNumBytesPerVertex();
            DWORD dwDestStride = sizeof(CTMfloat) * 3;

            for (DWORD dwVertex = 0; dwVertex < dwVertexCount; ++dwVertex)
            {
                float* pFloat = (float*) pSource;
                *pFloat = -*pFloat;
                //*pFloat = -*pFloat;
                pSource += dwSourceStride;
            }
        }

        g_pMesh->UnlockVertexBuffer();
    }

    // Lock down the indices and convert them to unsigned 32-bit integers.
    {
        void* pData = 0;
        g_pMesh->LockIndexBuffer(0, &pData);

        DWORD dwOptions = g_pMesh->GetOptions();
        DWORD dwSourceStride = (dwOptions & D3DXMESH_32BIT) ? 4 : 2;

        unsigned char* pSource = (unsigned char*) pData;

        for (DWORD dwIndex = 0; dwIndex < dwIndexCount / 3; ++dwIndex)
        {
            unsigned short* inds = (unsigned short*) pSource;
            
            std::swap(inds[0], inds[1]);

            pSource += dwSourceStride * 3;
        }

        g_pMesh->UnlockIndexBuffer();
    }


    D3DXSaveMeshToX(L"new.x", g_pMesh, 0, d3dxMaterials, d3dxEffects, g_dwNumMaterials, D3DXF_FILEFORMAT_BINARY | D3DXF_FILEFORMAT_COMPRESSED);
    cout << "Saved." << endl;
*/   
}

void Cleanup()
{
    if (g_pMeshMaterials)
        delete[] g_pMeshMaterials;

    if (g_pMeshTextures)
    {
        for (DWORD i = 0; i < g_dwNumMaterials; i++)
        {
            if (g_pMeshTextures[i])
                g_pMeshTextures[i]->Release();
        }
        delete[] g_pMeshTextures;
    }

    if (g_pMesh)
        g_pMesh->Release();

    if (g_pd3dDevice)
        g_pd3dDevice->Release();

    if (g_pD3D)
        g_pD3D->Release();
}

void Render()
{
    g_pd3dDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);
    g_pd3dDevice->BeginScene();

    D3DXMATRIXA16 matWorld;
    D3DXMatrixRotationY(&matWorld, timeGetTime() / 1000.0f);
    g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);

    D3DXVECTOR3 vEyePt(0.0f, 3.0f,-5.0f);
    D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
    D3DXMATRIXA16 matView;
    D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
    g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);

    D3DXMATRIXA16 matProj;
    D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f);
    g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);

    for (DWORD i = 0; i < g_dwNumMaterials; i++)
    {
        g_pd3dDevice->SetMaterial(&g_pMeshMaterials[i]);
        g_pd3dDevice->SetTexture(0, g_pMeshTextures[i]);
        g_pMesh->DrawSubset(i);
    }

    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present(0, 0, 0, 0);
}

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_DESTROY:
            Cleanup();
            PostQuitMessage(0);
            return 0;
        case WM_KEYDOWN:
            switch (wParam)
            {
                case VK_ESCAPE:
                    Cleanup();
                    PostQuitMessage(0);
                    break;
                case VK_OEM_2: // Question Mark / Forward Slash for US Keyboards
                    break;
            }
            break;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}
