
-- Vertex

in vec4 Position;
in vec3 Normal;
out vec3 vNormal;
uniform mat4 ModelviewProjection;
uniform mat3 NormalMatrix;

void main()
{
    vNormal = NormalMatrix * Normal;
    gl_Position = ModelviewProjection * Position;
}

-- Vertex.Lines

in vec4 Position;
uniform mat4 ModelviewProjection;

void main()
{
    gl_Position = ModelviewProjection * Position;
}

-- Vertex.Quad

in vec4 Position;

void main()
{
    gl_Position = Position;
}

-- Geometry

layout(triangles_adjacency) in;
layout(triangle_strip, max_vertices = 12) out;
uniform float HalfWidth;
uniform float OverhangLength;
out float gDist;
out vec3 gSpine;

bool IsFront(vec3 A, vec3 B, vec3 C)
{
    float area = (A.x * B.y - B.x * A.y) + (B.x * C.y - C.x * B.y) + (C.x * A.y - A.x * C.y);
    return area > 0;
}

void EmitEdge(vec3 P0, vec3 P1)
{
    vec3  E = OverhangLength * vec3(P1.xy - P0.xy, 0);
    vec2  V = normalize(E.xy);
    vec3  N = vec3(-V.y, V.x, 0) * HalfWidth;
    vec3  S = -N;
    float D = HalfWidth;

    gSpine = P0;
    gl_Position = vec4(P0 + S - E, 1); gDist = +D; EmitVertex();
    gl_Position = vec4(P0 + N - E, 1); gDist = -D; EmitVertex();
    gSpine = P1;
    gl_Position = vec4(P1 + S + E, 1); gDist = +D; EmitVertex();
    gl_Position = vec4(P1 + N + E, 1); gDist = -D; EmitVertex();
    EndPrimitive();
}

void main()
{
    vec3 v0 = gl_in[0].gl_Position.xyz / gl_in[0].gl_Position.w;
    vec3 v1 = gl_in[1].gl_Position.xyz / gl_in[1].gl_Position.w;
    vec3 v2 = gl_in[2].gl_Position.xyz / gl_in[2].gl_Position.w;
    vec3 v3 = gl_in[3].gl_Position.xyz / gl_in[3].gl_Position.w;
    vec3 v4 = gl_in[4].gl_Position.xyz / gl_in[4].gl_Position.w;
    vec3 v5 = gl_in[5].gl_Position.xyz / gl_in[5].gl_Position.w;
    
    if (IsFront(v0, v2, v4)) {
        if (!IsFront(v0, v1, v2)) EmitEdge(v0, v2);
        if (!IsFront(v2, v3, v4)) EmitEdge(v2, v4);
        if (!IsFront(v0, v4, v5)) EmitEdge(v4, v0);
    } 
}

-- Fragment.Lighting

out vec4 FragColor;

uniform sampler2D DepthSampler;
uniform sampler2D NormalMap;

uniform vec2 Size;
uniform vec3 LightPosition;
uniform vec3 DiffuseMaterial;
uniform vec3 AmbientMaterial;
uniform vec3 SpecularMaterial;
uniform float Shininess;

void main()
{
    vec2 texCoord = gl_FragCoord.xy / Size;
    float depth = texture2D(DepthSampler, texCoord).r;
    if (depth > 0.5)
        discard;

    vec3 vNormal = texture2D(NormalMap, texCoord).rgb;
    vNormal = (2.0 * vNormal) - vec3(1.0);

    vec3 N = normalize(vNormal);
    vec3 L = normalize(LightPosition);
    vec3 E = vec3(0, 0, 1);
    vec3 H = normalize(L + E);

    float df = max(0.0, dot(N, L));
    float sf = max(0.0, dot(N, H));
    sf = pow(sf, Shininess);
    vec3 color = AmbientMaterial + df * DiffuseMaterial + sf * SpecularMaterial;
    FragColor = vec4(color, 1.0);
}

-- Fragment.WriteNormals

in vec3 vNormal;
out vec3 FragColor;

void main()
{
    FragColor = 0.5 * (vNormal + vec3(1.0));
}

-- Fragment.Black

in float gDist;
in vec3 gSpine;
out vec4 FragColor;
uniform float HalfWidth;
uniform sampler2D DepthSampler;
uniform vec2 Size;

void main()
{
    vec2 texCoord = (gSpine.xy + 1.0) / 2.0;
    float depth = texture2D(DepthSampler, texCoord).r;
    if (depth < gl_FragCoord.z)
        discard;

    float alpha = 1.0;
    float d = abs(gDist);
    float tipLength = 2.0 * fwidth(d);
    if (d > HalfWidth - tipLength)
        alpha = 1.0 - (d - HalfWidth + tipLength) / tipLength;

    FragColor = vec4(0, 0, 0, alpha);
}
