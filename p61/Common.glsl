-- VS

attribute vec4 Position;
uniform mat4 ModelviewProjection;

void main()
{
    gl_Position = ModelviewProjection * Position;
}

-- Text.FS

uniform sampler2D Sampler;
uniform vec2 InverseSize;

void main()
{
    gl_FragColor = texture2D(Sampler, gl_FragCoord.xy * InverseSize);
}

-- Color.GS

#extension GL_EXT_geometry_shader4 : enable

void main()
{
    gl_Position = gl_PositionIn[0]; EmitVertex();
    gl_Position = gl_PositionIn[1]; EmitVertex();
    EndPrimitive();
}

-- Color.FS

uniform vec4 Color;

void main()
{
    gl_FragColor = Color;
}

-- LineStrip.VS

attribute vec4 Position;
attribute vec3 Normal;
attribute float PathCoord;
uniform mat4 ModelviewProjection;
varying vec3 vPosition;
varying vec3 vNormal;
varying float vPathCoord;

void main()
{
    gl_Position = ModelviewProjection * Position;
    vPosition = Position.xyz;
    vNormal = Normal;
    vPathCoord = PathCoord;
}

-- Overlay.VS

attribute vec4 Position;

void main()
{
    gl_Position = Position;
}

-- Overlay.FS

uniform sampler2D Sampler;
uniform sampler2D GradientSampler;
uniform vec2 InverseSize;

void main()
{
    float L = 1.0 - texture2D(Sampler, gl_FragCoord.xy * InverseSize).r;
    gl_FragColor = texture2D(GradientSampler, vec2(L, 0));
    gl_FragColor.rgb *= 0.5 * gl_FragColor.a;
    gl_FragColor += gl_FragColor;
}
