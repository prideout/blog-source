static const char* SimpleFragmentShader = STRINGIFY(

varying vec3 EyespaceNormal;
varying vec3 Diffuse;

uniform vec3 LightPosition;
uniform vec3 AmbientMaterial;
uniform vec3 SpecularMaterial;
uniform float Shininess;

void main(void)
{
    vec3 N = normalize(EyespaceNormal);
    vec3 L = normalize(LightPosition);
    vec3 E = vec3(0, 0, 1);
    vec3 H = normalize(L + E);
    
    float df = max(0.0, dot(N, L));
    float sf = max(0.0, dot(N, H));
    sf = pow(sf, Shininess);
    
    if (df < 0.1) df = 0.0;
    else if (df < 0.3) df = 0.3;
    else if (df < 0.6) df = 0.6;
    else df = 1.0;
    
    sf = step(0.5, sf);

    vec3 color = AmbientMaterial + df * Diffuse + sf * SpecularMaterial;

    gl_FragColor = vec4(color, 1);
}
);
