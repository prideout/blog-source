-- GS

#extension GL_EXT_geometry_shader4 : enable

varying in vec3 vPosition[4];
varying in vec3 vNormal[4];
varying in float vPathCoord[4];
varying out vec2 gPosition;
varying out vec2 gEndpoints[2];
varying out float gPathCoord;
uniform float Radius;
uniform mat4 Modelview;
uniform mat4 Projection;
uniform mat4 ModelviewProjection;
uniform float Time;

vec4 obb[8];
float pathCoords[2];

float compute_alpha(float pathCoord)
{
    float t = 2.0 * abs(pathCoord - mod(Time, 1.0));
    if (t > 1.0) t = 2.0 - t;
    t = max(0.0, 100.0 * (t - 0.99));
    return t;
}

vec2 to_screen(vec3 v)
{
    v = (Modelview * vec4(v, 1)).xyz;
    vec4 u = Projection * vec4(v, 1.0);
    return u.xy / u.w;
}

void emit()
{
    gPosition = gl_Position.xy / gl_Position.w;
    EmitVertex();
}

void emit(int a, int b, int c, int d)
{
    gPathCoord = pathCoords[a/4]; gl_Position = obb[a]; emit();
    gPathCoord = pathCoords[b/4]; gl_Position = obb[b]; emit();
    gPathCoord = pathCoords[c/4]; gl_Position = obb[c]; emit();
    gPathCoord = pathCoords[d/4]; gl_Position = obb[d]; emit();
}

void main()
{
    // Pass inputs to fragment shader:
    gEndpoints[0] = to_screen(vPosition[1]);
    gEndpoints[1] = to_screen(vPosition[2]);
    pathCoords[0] = compute_alpha(vPathCoord[1]);
    pathCoords[1] = compute_alpha(vPathCoord[2]);

    // Cull transparent segments:
    if (pathCoords[0] == 0.0 && pathCoords[1] == 0.0)
        return;

    // Compute normals for the connecting faces:
    vec3 p0, p1, p2, p3;
    p0 = vPosition[0]; p1 = vPosition[1];
    p2 = vPosition[2]; p3 = vPosition[3];
    vec3 n0 = normalize(p1-p0);
    vec3 n1 = normalize(p2-p1);
    vec3 n2 = normalize(p3-p2);
    vec3 u = normalize(n0+n1);
    vec3 v = normalize(n1+n2);

    // Compute the eight corners:

    vec3 i,j,k; float r = Radius;

    j = u; i = vNormal[1]; k = cross(i, j);
    obb[0] = ModelviewProjection*vec4(p1 + i*r + k*r,1);
    obb[1] = ModelviewProjection*vec4(p1 + i*r - k*r,1);
    obb[2] = ModelviewProjection*vec4(p1 - i*r - k*r,1);
    obb[3] = ModelviewProjection*vec4(p1 - i*r + k*r,1);

    j = v; i = vNormal[2]; k = cross(i, j);
    obb[4] = ModelviewProjection*vec4(p2 + i*r + k*r,1);
    obb[5] = ModelviewProjection*vec4(p2 + i*r - k*r,1);
    obb[6] = ModelviewProjection*vec4(p2 - i*r - k*r,1);
    obb[7] = ModelviewProjection*vec4(p2 - i*r + k*r,1);

    // Emit the connecting faces:
    emit(0,1,3,2); EndPrimitive();
    emit(5,4,6,7); EndPrimitive();
    emit(4,5,0,1); EndPrimitive();
    emit(3,2,7,6); EndPrimitive();
    emit(0,3,4,7); EndPrimitive();
    emit(2,1,6,5); EndPrimitive();
}

-- FS

varying vec2 gEndpoints[2];
varying vec2 gPosition;
varying float gPathCoord;
uniform float Radius;
uniform mat4 Projection;

float line_distance(vec2 pt, vec2 a, vec2 b)
{
    float dist = distance(a,b);
    vec2 v = normalize(b-a);
    float t = dot(v,pt-a);
    vec2 spinePoint;
    if (t > dist) spinePoint = b;
    else if (t > 0.0) spinePoint = a + t*v;
    else spinePoint = a;
    return distance(pt,spinePoint);
}

void main()
{
    vec2 x1 = gEndpoints[0];
    vec2 x2 = gEndpoints[1];
    float d = line_distance(gPosition,x1,x2);
    d = 1.0 - 12.0 * d;
    gl_FragColor = gPathCoord * vec4(vec3(d), 1.0);
}
