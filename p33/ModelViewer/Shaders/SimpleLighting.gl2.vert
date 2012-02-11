static const char* SimpleVertexShader = STRINGIFY(

attribute vec4 Position;
attribute vec3 Normal;
attribute vec3 DiffuseMaterial;

uniform vec3 AmbientMaterial;
uniform vec3 SpecularMaterial;
uniform float Shininess;
uniform mat4 Projection;
uniform mat4 Modelview;
uniform mat3 NormalMatrix;
uniform vec3 LightPosition;

varying vec4 FrontColor;

void main(void)
{
    vec3 normal = NormalMatrix * Normal;
    vec3 vp = normalize(LightPosition);
    float df = clamp(dot(normal, vp), 0.0, 1.0);
    
    vec3 hhat = normalize(vp + vec3(0, 0, 1));
    float sf = pow(clamp(dot(normal, hhat), 0.0, 1.0), Shininess);

    vec3 diffuse = df * DiffuseMaterial;
    vec3 specular = sf * SpecularMaterial;
    vec3 color = AmbientMaterial + diffuse + specular;
    
    FrontColor = vec4(color, 1);
    gl_Position = Projection * Modelview * Position;
}
);
