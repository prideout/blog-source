
-- VS

in vec4 Position;
out vec4 vPosition;
uniform mat4 ModelviewProjection;

void main()
{
    gl_Position = ModelviewProjection * Position;
    vPosition = Position;
}

-- GS

layout(points) in;
layout(line_strip, max_vertices = 2) out;
out float gAlpha;
uniform mat4 ModelviewProjection;
in vec4 vPosition[1];
uniform sampler3D Volume;

void main()
{
    vec3 coord = 0.5 * (vPosition[0].xyz + 1.0);
    vec4 V = vec4(texture(Volume, coord).xyz, 0.0);

    gAlpha = 0;
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();

    gAlpha = 1;
    gl_Position = ModelviewProjection * (vPosition[0] + V);
    EmitVertex();

    EndPrimitive();
}

-- FS

out vec4 FragColor;
in float gAlpha;
uniform float Brightness = 0.5;

void main()
{
    FragColor = Brightness * vec4(gAlpha, 0, 0, 1);
}
