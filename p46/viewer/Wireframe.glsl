-- vertex shader
in vec4 Position;

uniform mat4 Projection;
uniform mat4 Modelview;
void main()
{
    gl_Position = Projection * Modelview * Position;
}

-- geometry shader

out vec2 EdgeDistance;

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

void main()
{
    EdgeDistance = vec2(0, 1);
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();

    EdgeDistance = vec2(0, 0);
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();

    EdgeDistance = vec2(1, 0);
    gl_Position = gl_in[2].gl_Position;
    EmitVertex();

    EndPrimitive();
}

-- fragment shader

in vec2 EdgeDistance;
out vec4 FragColor;
const float Scale = 20.0;
const float Offset = -0.5;
uniform vec4 FillColor;

void main()
{
    float d = min(EdgeDistance.x, EdgeDistance.y);
    d = Scale * d + Offset;
    d = clamp(d, 0, 1);
    d = 1 - exp2(-2*d*d);
    FragColor = d*FillColor;
}
