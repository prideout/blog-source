-- VS

in vec4 Position;
out vec4 vPosition;
uniform mat4 ModelviewProjection;

void main()
{
    gl_Position = ModelviewProjection * Position;
    vPosition = Position;
}

-- Fullscreen

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in vec4 vPosition[1];
out vec2 gTexCoord;

void main()
{
    gTexCoord = vec2(1, 0); gl_Position = vec4(+1, -1, 0, 1); EmitVertex();
    gTexCoord = vec2(1, 1); gl_Position = vec4(+1, +1, 0, 1); EmitVertex();
    gTexCoord = vec2(0, 0); gl_Position = vec4(-1, -1, 0, 1); EmitVertex();
    gTexCoord = vec2(0, 1); gl_Position = vec4(-1, +1, 0, 1); EmitVertex();
    EndPrimitive();
}

-- Cube

layout(points) in;
layout(triangle_strip, max_vertices = 24) out;

in vec4 vPosition[1];
out vec3 gObj;

uniform mat4 ModelviewProjection;
uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 Modelview;

vec4 objCube[8]; // Object space coordinate of cube corner
vec4 ndcCube[8]; // Normalized device coordinate of cube corner
ivec4 faces[6];  // Vertex indices of the cube faces

void emit_vert(int vert)
{
    gObj = objCube[vert].xyz;
    gl_Position = ndcCube[vert];
    EmitVertex();
}

void emit_face(int face)
{
    emit_vert(faces[face][1]); emit_vert(faces[face][0]);
    emit_vert(faces[face][3]); emit_vert(faces[face][2]);
    EndPrimitive();
}

void main()
{
    faces[0] = ivec4(0,1,3,2); faces[1] = ivec4(5,4,6,7);
    faces[2] = ivec4(4,5,0,1); faces[3] = ivec4(3,2,7,6);
    faces[4] = ivec4(0,3,4,7); faces[5] = ivec4(2,1,6,5);

    vec4 P = vPosition[0];
    vec4 I = vec4(1,0,0,0);
    vec4 J = vec4(0,1,0,0);
    vec4 K = vec4(0,0,1,0);

    objCube[0] = P+K+I+J; objCube[1] = P+K+I-J;
    objCube[2] = P+K-I-J; objCube[3] = P+K-I+J;
    objCube[4] = P-K+I+J; objCube[5] = P-K+I-J;
    objCube[6] = P-K-I-J; objCube[7] = P-K-I+J;

    // Transform the corners of the box:
    for (int vert = 0; vert < 8; vert++)
        ndcCube[vert] = ModelviewProjection * objCube[vert];

    // Emit the faces:
    for (int face = 0; face < 6; face++)
        emit_face(face);
}

-- Intervals

out vec4 FragColor;
in vec3 gObj;

void main()
{
    FragColor = vec4(gObj, 1);
}

-- Raycast

out vec4 FragColor;
in vec2 gTexCoord;

uniform sampler2D RayStartPoints;
uniform sampler2D RayStopPoints;
uniform sampler3D Density;

uniform vec3 LightPosition = vec3(0.25,1.0,3);
uniform vec3 LightIntensity = vec3(15);
uniform float Absorption = 1.0;
uniform mat4 Modelview;
uniform mat4 ViewMatrix;

const float maxDist = sqrt(2.0);
const int numSamples = 128;
const float stepSize = maxDist/float(numSamples);
const int numLightSamples = 32;
const float lscale = maxDist / float(numLightSamples);
const float densityFactor = 5;

void main()
{
    vec3 rayStart = texture(RayStartPoints, gTexCoord).xyz;
    vec3 rayStop = texture(RayStopPoints, gTexCoord).xyz;

    if (rayStart == rayStop)
    {
        FragColor = vec4(1);
        return;
    }

    rayStart = 0.5 * (rayStart + 1.0);
    rayStop = 0.5 * (rayStop + 1.0);

    vec3 pos = rayStart;
    vec3 step = normalize(rayStop-rayStart) * stepSize;
    float travel = distance(rayStop, rayStart);
    float T = 1.0;
    vec3 Lo = vec3(0.0);

    for (int i=0; i < numSamples && travel > 0.0; ++i, pos += step, travel -= stepSize) {

        float density = texture(Density, pos).x * densityFactor;
        if (density <= 0.0) {
            continue;
        }
        T *= 1.0-density*stepSize*Absorption;
        if (T <= 0.01)
            break;

        vec3 lightDir = normalize(LightPosition-pos)*lscale;
        float Tl = 1.0;
        vec3 lpos = pos + lightDir;

        for (int s=0; s < numLightSamples; ++s) {
            float ld = texture(Density, lpos).x;
            Tl *= 1.0-Absorption*lscale*ld;
            if (Tl <= 0.01) 
            lpos += lightDir;
        }

        vec3 Li = LightIntensity*Tl;
        Lo += Li*T*density*stepSize;
    }

    FragColor.rgb = Lo;
    FragColor.a = 1-T;
}
