#pragma warning (disable:4996)

extern "C" {
    #include "lualib.h"
    #include "lauxlib.h"
}

#include <iostream>

using namespace std;

#define PATH(s) "../2010.04.20/" s

void First()
{
    lua_State* L = lua_open();
    luaL_openlibs(L);

    luaL_dofile(L, PATH("First.lua"));

    lua_getglobal(L, "MyVertexShader");
    lua_getglobal(L, "MyFragmentShader");
    const char* vs_text = lua_tostring(L, -2);
    const char* fs_text = lua_tostring(L, -1);
    lua_pop(L, 2);

    if (!vs_text)
    {
        cerr << "Can't find variable.";
        exit(1);
    }
        
    cout << "Vertex Shader" << endl;
    cout << vs_text << endl;

    cout << "Fragment Shader" << endl;
    cout << fs_text << endl;

    lua_close(L);
}

void Second()
{
    // Create the Lua context
    lua_State* L = lua_open();

    // Open Lua's utility libraries (including the debug library)
    luaL_openlibs(L);

# if 0
    // Run the script that defines the DeclareShader function:
    luaL_dofile(L, "ShaderFactory.lua");

    // Load our effect:
    luaL_dofile(L, "MyEffect.lua");
#else
    luaL_dofile(L, PATH("Second.lua"));
#endif

    // As before, extract strings and use them:
    lua_getglobal(L, "MyVertexShader");
    lua_getglobal(L, "MyFragmentShader");

    const char* vs_text = lua_tostring(L, -2);
    const char* fs_text = lua_tostring(L, -1);
    lua_pop(L, 2);

    cout << "Vertex Shader" << endl;
    cout << vs_text << endl;

    cout << "Fragment Shader" << endl;
    cout << fs_text << endl;

    lua_close(L);
}

const char* GetShaderSource(lua_State* L, const char* techniqueName,
                            const char* apiVersion, const char* shaderStage)
{
    // Append "Shaders" to the shader stage to obtain the table name:
    char tableName[32];
    strncpy(tableName, shaderStage, 24);
    strncat(tableName, "Shaders", 7);

    // Fetch the table from the Lua context and make sure it exists:
    lua_getglobal(L, tableName);
    if (!lua_istable(L, -1))
        return 0;

    // Make sure a table exists for the given API version:
    lua_pushstring(L, apiVersion);
    lua_gettable(L, -2);
    if (!lua_istable(L, -1))
        return 0;

    // Fetch the shader string:
    lua_pushstring(L, techniqueName);
    lua_gettable(L, -2);
    if (!lua_isstring(L, -1))
        return 0;
    const char* shaderSource = lua_tostring(L, -1);

    // Clean up the Lua stack and return the string:
    lua_pop(L, 3);
    return shaderSource;
}

void Third()
{
    // Create the Lua context
    lua_State* L = lua_open();

    // Open Lua's utility libraries (including the debug library)
    luaL_openlibs(L);

# if 0
    // Run the script that defines the DeclareShader function:
    luaL_dofile(L, "ShaderFactory.lua");

    // Load our effect:
    luaL_dofile(L, "MyEffect.lua");
#else
    if (luaL_dofile(L, PATH("Third.lua")))
    {
        const char* message = lua_tostring(L, -1);
        exit(1);
    }
#endif

    cout << "Vertex Shader 1:" << endl;
    cout << GetShaderSource(L, "SnazzyEffect", "GL2", "Vertex") << endl << endl;

    cout << "Vertex Shader 2:" << endl;
    cout << GetShaderSource(L, "SnazzyEffect", "GL3", "Vertex") << endl << endl;

    lua_close(L);
}

const char* GetShaderSource(lua_State* L, const char* effectKey)
{
    // Extract the effect path:
    const char* targetKey = strchr(effectKey, '.');
    if (!targetKey++)
        return 0;

    char effectName[32] = {0};
    strncpy(effectName, effectKey, targetKey - effectKey - 1);

    // Fetch the table from the Lua context and make sure it exists:
    lua_getglobal(L, effectName);
    if (!lua_istable(L, -1))
    {
        lua_pop(L, 1);

        // Delay-load the Lua file:
        char effectPath[64];
        sprintf(effectPath, PATH("%s.lua"), effectName);
        if (luaL_dofile(L, effectPath))
        {
            const char* message = lua_tostring(L, -1);
            return 0;
        }
        
        // If it's still not there, give up!
        lua_getglobal(L, effectName);
        if (!lua_istable(L, -1))
            exit(1);
    }

    const char* closestMatch = 0;
    int closestMatchLength = 0;

    int i = lua_gettop(L);
    lua_pushnil(L);
    while (lua_next(L, i) != 0)
    {
        const char* shaderKey = lua_tostring(L, -2);
        int shaderKeyLength = strlen(shaderKey);

        // Find the longest shader key that matches the first few letters of the target shader key.
        if (strstr(targetKey, shaderKey) != 0 && shaderKeyLength > closestMatchLength)
        {
            closestMatchLength = shaderKeyLength;
            closestMatch = lua_tostring(L, -1);
        }

        lua_pop(L, 1);
    }

    lua_pop(L, 1);

    return closestMatch;
}

void Four()
{
    lua_State* L = lua_open();
    luaL_openlibs(L);

    cout << "Vertex Shader 1:" << endl;
    cout << GetShaderSource(L, "Four.Vertex.GL3.KirkEffect") << endl << endl;

    cout << "Vertex Shader 2:" << endl;
    cout << GetShaderSource(L, "Four.Fragment.GL3.KirkEffect") << endl << endl;

    lua_close(L);
}

int main()
{
    Four();
}
