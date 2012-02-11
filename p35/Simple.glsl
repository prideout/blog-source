
-- Vertex.GL3

in vec2 Position;
in vec3 InColor;
out vec3 OutColor;

void main()
{
    OutColor = InColor;
    gl_Position = vec4(Position, 0, 1);
}

-- Fragment.GL3

in vec3 OutColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(OutColor, 1);
}
