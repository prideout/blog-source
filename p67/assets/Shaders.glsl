-- Advect.VS

in vec3 Position;
in float BirthTime;
in vec3 Velocity;

out vec3 vPosition;
out float vBirthTime;
out vec3 vVelocity;

uniform sampler3D Sampler;
uniform vec3 Size;
uniform vec3 Extent;
uniform float Time;
uniform float TimeStep = 5.0;
uniform float InitialBand = 0.1;
uniform float SeedRadius = 0.25;
uniform float PlumeCeiling = 3.0;
uniform float PlumeBase = -3;

const float TwoPi = 6.28318530718;
const float UINT_MAX = 4294967295.0;

uint randhash(uint seed)
{
    uint i=(seed^12345391u)*2654435769u;
    i^=(i<<6u)^(i>>26u);
    i*=2654435769u;
    i+=(i<<5u)^(i>>12u);
    return i;
}

float randhashf(uint seed, float b)
{
    return float(b * randhash(seed)) / UINT_MAX;
}

vec3 SampleVelocity(vec3 p)
{
    vec3 tc;
    tc.x = (p.x + Extent.x) / (2 * Extent.x);
    tc.y = (p.y + Extent.y) / (2 * Extent.y);
    tc.z = (p.z + Extent.z) / (2 * Extent.z);
    return texture(Sampler, tc).xyz;
}

void main()
{
    vPosition = Position;
    vBirthTime = BirthTime;

    // Seed a new particle as soon as an old one dies:
    if (BirthTime == 0.0 || Position.y > PlumeCeiling) {
        uint seed = uint(Time * 1000.0) + uint(gl_VertexID);
        float theta = randhashf(seed++, TwoPi);
        float r = randhashf(seed++, SeedRadius);
        float y = randhashf(seed++, InitialBand);
        vPosition.x = r * cos(theta);
        vPosition.y = PlumeBase + y;
        vPosition.z = r * sin(theta);
        vBirthTime = Time;
    }

    // Move the particles forward using a half-step to reduce numerical issues:
    vVelocity = SampleVelocity(Position);
    vec3 midx = Position + 0.5f * TimeStep * vVelocity;
    vVelocity = SampleVelocity(midx);
    vPosition += TimeStep * vVelocity;
}

-- Blit.VS

in vec4 Position;
in vec2 TexCoord;
out vec2 vTexCoord;
uniform float ScrollOffset;
uniform float Depth;

void main()
{
    vTexCoord = TexCoord;
    gl_Position = Position;
    gl_Position.z = Depth;
}

-- Blit.FS

out vec4 FragColor;
in vec2 vTexCoord;
uniform sampler2D Sampler;

void main()
{
    FragColor = texture(Sampler, vTexCoord);
}

-- Floor.VS

in vec4 Position;
in vec3 Normal;
out vec3 vPosition;
out vec3 vNormal;
out vec2 vTexCoord;
uniform mat4 ModelviewProjection;

void main()
{
    vTexCoord = Position.xy;
    vNormal = Normal;
    vPosition = Position.xyz;
    gl_Position = ModelviewProjection * Position;
}

-- Floor.FS

in vec3 vNormal;
in vec2 vTexCoord;

uniform sampler2D Sampler;

void main()
{
    gl_FragColor = texture(Sampler, vTexCoord);
}

-- Particle.VS

in vec4 Position;
in vec3 Velocity;
in float BirthTime;
uniform mat4 ModelviewProjection;
uniform float Time;
out float vAlpha;
out float vBirthTime;
out vec3 vVelocity;
out vec3 vPosition;
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

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform mat3 Modelview;
uniform mat4 ModelviewProjection;
uniform float PointSize;

in vec3 vPosition[1];
in vec3 vVelocity[1];
in float vAlpha[1];
in float vBirthTime[1];
out float gAlpha;
out vec2 gTexCoord;

const float Epsilon = 0.0000001;

void main()
{
    if (vBirthTime[0] == 0.0)
        return;

    vec3 u = Modelview * vVelocity[0];
    float w = PointSize;
    float h = w * 5.0;

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
in float gAlpha;
in vec2 gTexCoord;
out vec4 FragColor;
uniform sampler2D SpriteSampler;
uniform sampler2D DepthSampler;

void main()
{
    vec2 tc = gl_FragCoord.xy * InverseSize;
    float depth = texture(DepthSampler, tc).r;
    if (depth < gl_FragCoord.z)
        discard;

    float d = depth - gl_FragCoord.z;
    float softness = 1.0 - min(1.0, 40.0 * d);
    softness *= softness;
    softness = 1.0 - softness * softness;

    float falloff = texture(SpriteSampler, gTexCoord).r;
    float A = gAlpha * falloff * falloff;
    FragColor = Color * vec4(1, 1, 1, A * softness);
}

-- Composite.VS

in vec4 Position;
in vec2 TexCoord;
out vec2 vTexCoord;
uniform float ScrollOffset;
uniform float Depth;

void main()
{
    vTexCoord = TexCoord;
    gl_Position = Position;
    gl_Position.z = Depth;
}

-- Composite.FS

in vec2 vTexCoord;
out vec4 FragColor;
uniform sampler2D BackgroundSampler;
uniform sampler2D ParticlesSampler;

void main()
{
    vec2 tc = vTexCoord; tc.y = 1.0 - tc.y;
    vec4 dest = texture(BackgroundSampler, tc);
    vec4 src = texture(ParticlesSampler, tc);
    float a = 1.0 - src.a;
    FragColor.rgb = src.rgb * a + dest.rgb * (1.0 - a);
    FragColor.a = 1.0;
}
