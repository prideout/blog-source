
-- Vertex

attribute vec4 Position;
attribute vec3 Normal;
varying vec3 vNormal;
uniform mat4 ModelviewProjection;
uniform mat3 NormalMatrix;

void main()
{
    vNormal = NormalMatrix * Normal;
    gl_Position = ModelviewProjection * Position;
}


-- Vertex.Bent

attribute vec3 Position;
attribute vec3 Normal;
varying vec3 vNormal;
uniform mat4 ModelviewProjection;
uniform mat3 NormalMatrix;
uniform sampler2D Sampler;
uniform float InverseWidth;
uniform float InverseHeight;

#define TextureHeight 128

uniform float PathOffset;
uniform float PathScale;

uniform int InstanceOffset;

void main()
{
    float id = gl_InstanceID + InstanceOffset;
    float xstep = InverseWidth;
    float ystep = InverseHeight;
    float xoffset = 0.5 * xstep;
    float yoffset = 0.5 * ystep;

    // Look up the current and previous centerline positions:
    vec2 texCoord;
    texCoord.x = PathScale * Position.x + PathOffset + xoffset;
    texCoord.y = 2.0 * id / TextureHeight + yoffset;
    
    vec3 currentCenter = texture2D(Sampler, texCoord).rgb;
    vec3 previousCenter = texture2D(Sampler, texCoord - vec2(xstep, 0)).rgb;
    vec3 pathDirection = normalize(currentCenter - previousCenter);

    // Look up the current orientation vector:
    texCoord.x = PathOffset + xoffset;
    texCoord.y = texCoord.y + ystep;
    vec3 pathNormal = texture2D(Sampler, texCoord).rgb;

    // Form the change-of-basis matrix:
    vec3 a = pathDirection;
    vec3 b = pathNormal;
    vec3 c = cross(a, b);
    mat3 basis = mat3(a, b, c);

    // Transform the normal vector and positions:
    vNormal = NormalMatrix * (basis * Normal);
    vec3 position = currentCenter + (basis * vec3(0, Position.yz));
    gl_Position = ModelviewProjection * vec4(position, 1);
}

-- Fragment

varying vec3 vNormal;

uniform vec2 Size;
uniform vec3 LightPosition;
uniform vec3 DiffuseMaterial;
uniform vec3 AmbientMaterial;
uniform vec3 SpecularMaterial;
uniform float Shininess;

void main()
{
    vec3 N = normalize(vNormal);
    vec3 L = normalize(LightPosition);
    vec3 E = vec3(0, 0, 1);
    vec3 H = normalize(L + E);

    float df = max(0.0, dot(N, L));
    float sf = max(0.0, dot(N, H));
    sf = pow(sf, Shininess);
    vec3 color = AmbientMaterial + df * DiffuseMaterial + sf * SpecularMaterial;
    gl_FragColor = vec4(color, 1.0);
}
