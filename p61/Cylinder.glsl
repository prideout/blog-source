-- GS

#extension GL_EXT_geometry_shader4 : enable

varying in vec3 vPosition[4];
varying in vec3 vNormal[4];
varying out vec3 gPosition;
varying out vec3 gEndpoints[4];
varying out vec3 gEndplanes[2];
uniform float Radius;
uniform mat4 Modelview;
uniform mat4 Projection;

vec4 obb[8];
vec4 obbPrime[8];

bool isFront(int a, int b, int c)
{
    vec3 i = vec3(obbPrime[b].xy - obbPrime[a].xy, 0);
    vec3 j = vec3(obbPrime[c].xy - obbPrime[a].xy, 0);
    return cross(i, j).z > 0.0;
}

void emit(int a, int b, int c, int d)
{
    gPosition = obb[a].xyz; gl_Position = obbPrime[a]; EmitVertex();
    gPosition = obb[b].xyz; gl_Position = obbPrime[b]; EmitVertex();
    gPosition = obb[c].xyz; gl_Position = obbPrime[c]; EmitVertex();
    gPosition = obb[d].xyz; gl_Position = obbPrime[d]; EmitVertex();
}

void main()
{
    // Pass raytracing inputs to fragment shader:
    vec3 p0, p1, p2, p3, n0, n1, n2;
    p0 = (Modelview * vec4(vPosition[0], 1)).xyz;
    p1 = (Modelview * vec4(vPosition[1], 1)).xyz;
    p2 = (Modelview * vec4(vPosition[2], 1)).xyz;
    p3 = (Modelview * vec4(vPosition[3], 1)).xyz;
    n0 = normalize(p1-p0);
    n1 = normalize(p2-p1);
    n2 = normalize(p3-p2);
    gEndpoints[0] = p0; gEndpoints[1] = p1;
    gEndpoints[2] = p2; gEndpoints[3] = p3;
    gEndplanes[0] = normalize(n0+n1);
    gEndplanes[1] = normalize(n1+n2);

    // Compute object-space plane normals:
    p0 = vPosition[0]; p1 = vPosition[1];
    p2 = vPosition[2]; p3 = vPosition[3];
    n0 = normalize(p1-p0);
    n1 = normalize(p2-p1);
    n2 = normalize(p3-p2);
    vec3 u = normalize(n0+n1);
    vec3 v = normalize(n1+n2);

    // Generate a basis for the cuboid:
    vec3 j = n1;
    vec3 i = vNormal[1];
    vec3 k = cross(i, j);

    // Compute the eight corners:
    float r = Radius; float d;
    d = 1.0/dot(u,j); p1 -= j*r*sqrt(d*d-1.0);
    d = 1.0/dot(v,j); p2 += j*r*sqrt(d*d-1.0);
    obb[0] = Modelview*vec4(p1 + i*r + k*r,1);
    obb[1] = Modelview*vec4(p1 + i*r - k*r,1);
    obb[2] = Modelview*vec4(p1 - i*r - k*r,1);
    obb[3] = Modelview*vec4(p1 - i*r + k*r,1);
    obb[4] = Modelview*vec4(p2 + i*r + k*r,1);
    obb[5] = Modelview*vec4(p2 + i*r - k*r,1);
    obb[6] = Modelview*vec4(p2 - i*r - k*r,1);
    obb[7] = Modelview*vec4(p2 - i*r + k*r,1);
    for (int i = 0; i < 8; i++)
        obbPrime[i] = Projection * obb[i];
    
    // Emit the front faces of the cuboid:
    if (isFront(0,1,3)) emit(0,1,3,2); EndPrimitive();
    if (isFront(5,4,6)) emit(5,4,6,7); EndPrimitive();
    if (isFront(4,5,0)) emit(4,5,0,1); EndPrimitive();
    if (isFront(3,2,7)) emit(3,2,7,6); EndPrimitive();
    if (isFront(0,3,4)) emit(0,3,4,7); EndPrimitive();
    if (isFront(2,1,6)) emit(2,1,6,5); EndPrimitive();
}

-- FS

uniform vec4 Color;

varying vec3 gEndpoints[4];
varying vec3 gEndplanes[2];
varying vec3 gPosition;

uniform float Radius;
uniform mat4 Projection;
uniform vec3 LightDirection;
uniform vec3 DiffuseMaterial;
uniform vec3 AmbientMaterial;
uniform vec3 SpecularMaterial;
uniform float Shininess;

vec3 perp(vec3 v)
{
    vec3 b = cross(v, vec3(0, 0, 1));
    if (dot(b, b) < 0.01)
        b = cross(v, vec3(0, 1, 0));
    return b;
}

bool IntersectCylinder(vec3 origin, vec3 dir, out float t)
{
    vec3 A = gEndpoints[1]; vec3 B = gEndpoints[2];
    float Epsilon = 0.0000001;
    float extent = distance(A, B);
    vec3 W = (B - A) / extent;
    vec3 U = perp(W);
    vec3 V = cross(U, W);
    U = normalize(cross(V, W));
    V = normalize(V);
    float rSqr = Radius*Radius;
    vec3 diff = origin - 0.5 * (A + B);
    mat3 basis = mat3(U, V, W);
    vec3 P = diff * basis;
    float dz = dot(W, dir);
    if (abs(dz) >= 1.0 - Epsilon) {
        float radialSqrDist = rSqr - P.x*P.x - P.y*P.y;
        if (radialSqrDist < 0.0)
            return false;
        t = (dz > 0.0 ? -P.z : P.z) + extent * 0.5;
        return true;
    }

    vec3 D = vec3(dot(U, dir), dot(V, dir), dz);
    float a0 = P.x*P.x + P.y*P.y - rSqr;
    float a1 = P.x*D.x + P.y*D.y;
    float a2 = D.x*D.x + D.y*D.y;
    float discr = a1*a1 - a0*a2;
    if (discr < 0.0)
        return false;

    if (discr > Epsilon) {
        float root = sqrt(discr);
        float inv = 1.0/a2;
        t = (-a1 + root)*inv;
        return true;
    }

    t = -a1/a2;
    return true;
}

vec3 ComputeLight(vec3 L, vec3 N, bool specular)
{
    float df = max(0.0,dot(N, L));
    vec3 color = df * DiffuseMaterial;
    if (df > 0.0 && specular) {
        vec3 E = vec3(0, 0, 1);
        vec3 R = reflect(L, N);
        float sf = max(0.0,dot(R, E));
        sf = pow(sf, Shininess);
        color += sf * SpecularMaterial;
    }
    return color;
}

void main()
{
    vec3 rayStart = gPosition;
    vec3 rayEnd = vec3(0);
    vec3 rayDir = normalize(rayEnd - rayStart);

    if (distance(rayStart, rayEnd) < 0.1) {
        discard;
        return;
    }
    
    float d;
    if (!IntersectCylinder(rayStart, rayDir, d)) {
        discard;
        return;
    }

    vec3 hitPoint = rayStart + d * rayDir;
    if (dot(hitPoint - gEndpoints[1], gEndplanes[0]) < 0.0) {
        discard;
        return;
    }

    if (dot(hitPoint - gEndpoints[2], gEndplanes[1]) > 0.0) {
        discard;
        return;
    }

    // Compute a lighting normal:
    vec3 x0 = hitPoint;
    vec3 x1 = gEndpoints[1];
    vec3 x2 = gEndpoints[2];
    float length = distance(x1, x2);
    vec3 v = (x2 - x1) / length;
    float t = dot(x0 - x1, v);
    vec3 spinePoint = x1 + t * v;
    vec3 N = -normalize(hitPoint - spinePoint);

    // Perform lighting and write out a new depth value:
    vec3 color = AmbientMaterial + ComputeLight(LightDirection, N, true);
    vec4 ndc = Projection * vec4(hitPoint, 1);
    gl_FragDepth = ndc.z / ndc.w;
    gl_FragColor = vec4(color, 1.0);
}
