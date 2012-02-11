-- VS

attribute vec4 Position;
uniform mat4 ModelviewProjection;
varying vec3 vPosition;

void main()
{
    gl_Position = ModelviewProjection * Position;
    vPosition = Position.xyz;
}

-- GS

#extension GL_EXT_geometry_shader4 : enable

varying in vec3 vPosition[2];
varying out vec2 gCoord;

uniform mat4 ModelviewProjection;
uniform float Radius;
uniform mat3 Modelview;
uniform mat4 Projection;
uniform float Time;

float Epsilon = 0.001;

void main()
{
    vec3 p = mix(vPosition[0], vPosition[1], Time);
    float w = Radius * 0.5;
    float h = w * 2.0;
    vec3 u = Modelview * (vPosition[1] - vPosition[0]);

    // Determine 't', which represents Z-aligned magnitude.
    // By default, t = 0.0.
    // If velocity aligns with Z, increase t towards 1.0.
    // If total speed is negligible, increase t towards 1.0.
    float t = 0.0;
    float nz = abs(normalize(u).z);
    if (nz > 1.0 - Epsilon)
        t = (nz - (1.0 - Epsilon)) / Epsilon;
    else if (dot(u,u) < Epsilon)
        t = (Epsilon - dot(u,u)) / Epsilon;

    // Compute screen-space velocity:
    u.z = 0.0;
    u = normalize(u);

    // Lerp the orientation vector if screen-space velocity is negligible:
    u = normalize(mix(u, vec3(1,0,0), t));
    h = mix(h, w, t);

    // Compute the change-of-basis matrix for the billboard:
    vec3 v = vec3(-u.y, u.x, 0);
    vec3 a = u * Modelview;
    vec3 b = v * Modelview;
    vec3 c = cross(a, b);
    mat3 basis = mat3(a, b, c);

    // Compute the four offset vectors:
    vec3 N = basis * vec3(0,w,0);
    vec3 S = basis * vec3(0,-w,0);
    vec3 E = basis * vec3(-h,0,0);
    vec3 W = basis * vec3(h,0,0);

    // Emit the quad:
    gCoord = vec2(1,1); gl_Position = ModelviewProjection * vec4(p+N+E,1); EmitVertex();
    gCoord = vec2(-1,1); gl_Position = ModelviewProjection * vec4(p+N+W,1); EmitVertex();
    gCoord = vec2(1,-1); gl_Position = ModelviewProjection * vec4(p+S+E,1); EmitVertex();
    gCoord = vec2(-1,-1); gl_Position = ModelviewProjection * vec4(p+S+W,1); EmitVertex();
    EndPrimitive();
}

-- FS

varying vec2 gCoord;

void main()
{
    float r2 = dot(gCoord, gCoord);
    float d = exp(r2 * -1.2); // Gaussian Splat
    gl_FragColor = vec4(vec3(d), 1.0);
}
