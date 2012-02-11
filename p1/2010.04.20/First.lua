-- Vertex Shader
MyVertexShader = [[
in vec4 Position;
uniform mat4 Projection;
void main()
{
    gl_Position = Projection * Position;
}]]

-- Fragment Shader
MyFragmentShader = [[
out vec4 FragColor;
uniform vec4 FillColor;
void main()
{
    FragColor = FillColor;
}]]
