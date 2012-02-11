
-- VS

in vec4 Position;
out vec3 vPosition;
uniform mat4 ModelviewProjection;

void main()
{
    gl_Position = ModelviewProjection * Position;
    vPosition = Position.xyz;
}

-- GS

uniform mat4 Modelview;
uniform mat3 NormalMatrix;
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;
in vec3 vPosition[3];
out vec3 gFacetNormal;
out vec3 gTriDistance;

void main()
{
    vec3 A = vPosition[2] - vPosition[0];
    vec3 B = vPosition[1] - vPosition[0];
    gFacetNormal = NormalMatrix * normalize(cross(A, B));

    gTriDistance = vec3(1, 0, 0);
    gl_Position = gl_in[0].gl_Position; EmitVertex();

    gTriDistance = vec3(0, 1, 0);
    gl_Position = gl_in[1].gl_Position; EmitVertex();

    gTriDistance = vec3(0, 1, 1);
    gl_Position = gl_in[2].gl_Position; EmitVertex();

    EndPrimitive();
}

-- FS

out vec4 FragColor;
in vec3 gFacetNormal;
in vec3 gTriDistance;
uniform vec3 LightPosition;
uniform vec3 DiffuseMaterial;

float amplify(float d, float scale, float offset)
{
    d = scale * d + offset;
    d = clamp(d, 0, 1);
    d = 1 - exp2(-2*d*d);
    return d;
}

void main()
{
    vec3 N = normalize(gFacetNormal);
    vec3 L = normalize(LightPosition);
    float df = abs(dot(N, L));
    vec3 color = vec3(df);

    float d1 = min(min(gTriDistance.x, gTriDistance.y), gTriDistance.z);
    if (d1 > 0.05) {
        discard;
        return;
    }
    d1 = 1 - amplify(d1, 100, -0.5);
    color = color - d1 * vec3(1);
    
    if (!gl_FrontFacing) {
        discard;
        return;
    }
        
    FragColor = vec4(color, 1.0);
}
