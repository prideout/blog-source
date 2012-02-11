-- Vertex

in vec4 Position;
void main() {
    gl_Position = Position;
}

-- Fragment

uniform sampler2D Sampler;
out vec4 FragColor;
void main() {
    ivec2 coord = ivec2(gl_FragCoord.xy);
    FragColor = texelFetch(Sampler, coord, 0);
}
