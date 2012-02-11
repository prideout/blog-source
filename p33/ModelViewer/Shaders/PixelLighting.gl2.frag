static const char* SimpleFragmentShader = STRINGIFY(

varying vec3 EyespaceNormal;
varying vec3 Diffuse;

uniform vec3 LightPosition;
uniform vec3 AmbientMaterial;
uniform vec3 SpecularMaterial;
uniform float Shininess;

vec3 RoughNormalize(vec3 v)
{
    return 0.5 * v * (3.0 - dot(v,v));
}

void main(void)
{
    //vec3 N = normalize(EyespaceNormal);
    vec3 N = RoughNormalize(EyespaceNormal); // Does this really help FPS?
    vec3 L = normalize(LightPosition);
    vec3 E = vec3(0, 0, 1);
    vec3 H = normalize(L + E);
    
    float df = max(0.0, dot(N, L));
    float sf = max(0.0, dot(N, H));
    sf = pow(sf, Shininess);

    vec3 color = AmbientMaterial + df * Diffuse + sf * SpecularMaterial;

    gl_FragColor = vec4(color, 1);
}
);
