static const char* SimpleFragmentShader = STRINGIFY(

uniform sampler2D Sampler;

varying vec4 DestinationColor;
varying vec2 TextureCoordOut;

void main(void)
{
    gl_FragColor = texture2D(Sampler, TextureCoordOut) * DestinationColor;
}
);
