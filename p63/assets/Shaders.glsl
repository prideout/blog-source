-- Blit.VS

attribute vec4 Position;
attribute vec2 TexCoord;
varying vec2 vTexCoord;
uniform float ScrollOffset;
uniform float Depth;

void main()
{
    vTexCoord = TexCoord;
    vTexCoord.s = vTexCoord.s * 0.125 + ScrollOffset;
    gl_Position = Position;
    gl_Position.z = Depth;
}

-- Blit.FS

varying vec2 vTexCoord;
uniform sampler2D Sampler;

void main()
{
    gl_FragColor = texture2D(Sampler, vTexCoord);
}

-- Lit.VS

attribute vec4 Position;
attribute vec3 Normal;
varying vec3 vPosition;
varying vec3 vNormal;
uniform mat4 ModelviewProjection;

void main()
{
    vNormal = Normal;
    vPosition = Position.xyz;
    gl_Position = ModelviewProjection * Position;
}

-- Lit.GS

#extension GL_EXT_geometry_shader4 : enable

varying in vec3 vPosition[3];
varying out vec3 gFacetNormal;

void main()
{
    vec3 A = vPosition[2] - vPosition[0];
    vec3 B = vPosition[1] - vPosition[0];
    gFacetNormal = normalize(-cross(A, B));
    gl_Position = gl_PositionIn[0]; EmitVertex();
    gl_Position = gl_PositionIn[1]; EmitVertex();
    gl_Position = gl_PositionIn[2]; EmitVertex();
    EndPrimitive();
}

-- Lit.FS

//varying vec3 gFacetNormal;
varying vec3 vNormal;

uniform mat3 ViewMatrix;
uniform vec3 LightPosition;
uniform vec3 DiffuseMaterial;
uniform vec3 AmbientMaterial;
uniform vec3 SpecularMaterial;
uniform float Shininess;
uniform mat3 NormalMatrix;

vec3 ComputeLight(vec3 L, vec3 N, bool specular)
{
    L = ViewMatrix * normalize(L);
    float df = max(0.0,dot(N, L));
    vec3 color = df * DiffuseMaterial;
    if (df > 0.0 && specular) {
        vec3 E = vec3(0, 0, 1);
        vec3 R = reflect(-L, N);
        float sf = max(0.0,dot(R, E));
        sf = pow(sf, Shininess);
        color += sf * SpecularMaterial;
    }
    return color;
}

void main()
{
    //vec3 N = NormalMatrix * normalize(gFacetNormal);
    vec3 N = NormalMatrix * normalize(vNormal);

    vec3 color = AmbientMaterial;
    color += 0.5 * ComputeLight(LightPosition, N, true);

    vec3 cameraLight = vec3(0, 0, 1) * ViewMatrix;
    color += 0.5 * ComputeLight(cameraLight, N, false);

    gl_FragColor = vec4(color, 1.0);
}

-- Particle.VS

attribute vec4 Position;
attribute vec3 Velocity;
attribute float BirthTime;
uniform mat4 ModelviewProjection;
uniform float Time;
varying float vAlpha;
varying float vBirthTime;
varying vec3 vVelocity;
varying vec3 vPosition;
uniform float FadeRate;

void main()
{
    gl_Position = ModelviewProjection * Position;
    vBirthTime = BirthTime;
    vAlpha = max(0.0, 1.0 - (Time - BirthTime) * FadeRate);
    vVelocity = normalize(Velocity);
    vPosition = Position.xyz;
}

-- Particle.GS

#extension GL_EXT_geometry_shader4 : enable

uniform mat3 Modelview;
uniform mat4 ModelviewProjection;
uniform float PointSize;
varying in vec3 vPosition[1];
varying in vec3 vVelocity[1];
varying in float vAlpha[1];
varying in float vBirthTime[1];
varying out float gAlpha;
varying out vec2 gTexCoord;

const float Epsilon = 0.0000001;

void main()
{
    if (vBirthTime[0] == 0.0)
        return;

    vec3 u = Modelview * vVelocity[0];
    float w = PointSize;
    float h = w * 2.0;

    // Determine 't', which represents Z-aligned magnitude.
    // By default, t = 0.0.
    // If velocity aligns with Z, increase t towards 1.0.
    // If total speed is negligible, increase t towards 1.0.
    float t = 0.0;
    float nz = abs(normalize(u).z);
    if (nz > 1.0 - Epsilon)
        t = (nz - (1.0 - Epsilon)) / Epsilon;
    else if (dot(u,u) < Epsilon)
        t = (Epsilon - dot(u,u)) / Epsilon;

    // Compute screen-space velocity:
    u.z = 0.0;
    u = normalize(u);

    // Lerp the orientation vector if screen-space velocity is negligible:
    u = normalize(mix(u, vec3(1,0,0), t));
    h = mix(h, w, t);

    // Compute the change-of-basis matrix for the billboard:
    vec3 v = vec3(-u.y, u.x, 0);
    vec3 a = u * Modelview;
    vec3 b = v * Modelview;
    vec3 c = cross(a, b);
    mat3 basis = mat3(a, b, c);

    // Compute the four offset vectors:
    vec3 N = basis * vec3(0,w,0);
    vec3 S = basis * vec3(0,-w,0);
    vec3 E = basis * vec3(-h,0,0);
    vec3 W = basis * vec3(h,0,0);
    
    // Emit the quad:
    vec3 p = vPosition[0];
    gAlpha = vAlpha[0];
    gTexCoord = vec2(0,0); gl_Position = ModelviewProjection * vec4(p+N+W,1); EmitVertex();
    gTexCoord = vec2(1,0); gl_Position = ModelviewProjection * vec4(p+N+E,1); EmitVertex();
    gTexCoord = vec2(0,1); gl_Position = ModelviewProjection * vec4(p+S+W,1); EmitVertex();
    gTexCoord = vec2(1,1); gl_Position = ModelviewProjection * vec4(p+S+E,1); EmitVertex();
    EndPrimitive();
}

-- Particle.FS

uniform vec4 Color;
uniform vec2 InverseSize;
varying float gAlpha;
varying vec2 gTexCoord;
uniform sampler2D SpriteSampler;
uniform sampler2D DepthSampler;

void main()
{
    vec2 tc = gl_FragCoord.xy * InverseSize;
    float depth = texture2D(DepthSampler, tc).r;
    if (depth < gl_FragCoord.z)
        discard;

    float d = depth - gl_FragCoord.z;
    float softness = 1.0 - min(1.0, 40.0 * d);
    softness *= softness;
    softness = 1.0 - softness * softness;

    float A = gAlpha * texture2D(SpriteSampler, gTexCoord).r;
    gl_FragColor = Color * vec4(1, 1, 1, A * softness);
}

-- Composite.VS

attribute vec4 Position;
attribute vec2 TexCoord;
varying vec2 vTexCoord;
uniform float ScrollOffset;
uniform float Depth;

void main()
{
    vTexCoord = TexCoord;
    gl_Position = Position;
    gl_Position.z = Depth;
}

-- Composite.FS

varying vec2 vTexCoord;
uniform sampler2D BackgroundSampler;
uniform sampler2D ParticlesSampler;

void main()
{
    vec2 tc = vTexCoord; tc.y = 1.0 - tc.y;
    vec4 dest = texture2D(BackgroundSampler, tc);
    vec4 src = texture2D(ParticlesSampler, tc);
    float a = 1.0 - src.a;
    gl_FragColor.rgb = src.rgb * a + dest.rgb * (1.0 - a);
    gl_FragColor.a = 1.0;
}
