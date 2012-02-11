
-- Capsule.GS

#extension GL_EXT_geometry_shader4 : enable

varying in vec3 vPosition[4];

varying out vec3 gCapsuleA;
varying out vec3 gCapsuleB;
varying out vec2 gPosition;
varying out float gMinZ;
varying out float gMaxZ;

uniform float Radius;
uniform mat4 Modelview;
uniform mat4 Projection;
uniform mat4 ModelviewProjection;

vec3 perp(vec3 v)
{
    vec3 b = cross(v, vec3(0, 0, 1));
    if (dot(b, b) < 0.01)
        b = cross(v, vec3(0, 1, 0));
    return b;
}

vec3 eyespace(vec3 v)
{
    return (Modelview * vec4(v, 1)).xyz;
}

void main()
{
    // Generate a basis for the cuboid:
    vec3 j = normalize(vPosition[2] - vPosition[1]);
    vec3 i = perp(j);
    vec3 k = cross(i, j);
    
    float r = Radius;
    vec3 a = vPosition[1] - r * j;
    vec3 b = vPosition[2] + r * j;
    
    // Compute the eight corners in eyespace:
    vec3 obb[8];
    obb[0] = eyespace(a + i * r + k * r);
    obb[1] = eyespace(a + i * r - k * r);
    obb[2] = eyespace(a - i * r - k * r);
    obb[3] = eyespace(a - i * r + k * r);
    obb[4] = eyespace(b + i * r + k * r);
    obb[5] = eyespace(b + i * r - k * r);
    obb[6] = eyespace(b - i * r - k * r);
    obb[7] = eyespace(b - i * r + k * r);
    
    // Find the axis-aligned bounding box of the cuboid:
    vec3 aabb_min = obb[0];
    vec3 aabb_max = obb[0];
    for (int i = 1; i < 8; i++) {
        aabb_min = min(aabb_min, obb[i]);
        aabb_max = max(aabb_max, obb[i]);
    }
    float Z = aabb_min.z;

    // Give the fragment shader some values for raytracing:
    gCapsuleA = eyespace(vPosition[1]);
    gCapsuleB = eyespace(vPosition[2]);
    gMinZ = aabb_min.z;
    gMaxZ = aabb_max.z;

    // Emit the four corners of the AABB:
    gPosition = vec2(aabb_min.x, aabb_min.y);
    gl_Position = Projection * vec4(aabb_min.x, aabb_min.y, Z, 1);
    EmitVertex();

    gPosition = vec2(aabb_max.x, aabb_min.y);
    gl_Position = Projection * vec4(aabb_max.x, aabb_min.y, Z, 1);
    EmitVertex();

    gPosition = vec2(aabb_min.x, aabb_max.y);
    gl_Position = Projection * vec4(aabb_min.x, aabb_max.y, Z, 1);
    EmitVertex();

    gPosition = vec2(aabb_max.x, aabb_max.y);
    gl_Position = Projection * vec4(aabb_max.x, aabb_max.y, Z, 1);
    EmitVertex();

    EndPrimitive();
}

-- Sphere.FS

uniform vec4 Color;

varying vec3 gCapsuleA;
varying vec3 gCapsuleB;
varying vec2 gPosition;
varying float gMinZ;
varying float gMaxZ;

uniform float Radius;
uniform mat4 Modelview;
uniform mat4 Projection;
uniform mat4 ModelviewProjection;

uniform mat3 ViewMatrix;
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

bool IntersectSphere(vec3 rO, vec3 rD, vec3 sC, float sR, out float d)
{
    vec3 L = sC - rO;
    float s = dot(L, rD);
    float L2 = dot(L, L);
    float sqR = sR * sR;
    float m2 = L2 - s * s;
    float q = sqrt(sqR - m2);
    d = (L2 > sqR) ? (s - q) : (s + q);
    return d > 0.0;
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
    vec3 rayStart = vec3(gPosition, gMinZ);
    vec3 rayEnd = vec3(gPosition, gMaxZ);
    vec3 rayDir = normalize(rayEnd - rayStart);
    
    vec3 sphereCenter = 0.5 * (gCapsuleA + gCapsuleB);
    float d;
    if (!IntersectSphere(rayStart, rayDir, sphereCenter, Radius, d))
        discard;

    vec3 hitPoint = rayStart + d * rayDir;
    vec3 N = normalize(hitPoint - sphereCenter);
    vec3 color = AmbientMaterial + ComputeLight(LightDirection, N, true);
    vec4 ndc = Projection * vec4(hitPoint, 1.0);

    gl_FragDepth = ndc.z / ndc.w;
    gl_FragColor = vec4(color, 1.0);
}

-- Capsule.FS

uniform vec4 Color;

varying vec3 gCapsuleA;
varying vec3 gCapsuleB;
varying vec2 gPosition;
varying float gMinZ;
varying float gMaxZ;

uniform float Radius;
uniform mat4 Modelview;
uniform mat4 Projection;
uniform mat4 ModelviewProjection;

uniform mat3 ViewMatrix;
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

int IntersectCapsule(vec3 origin, vec3 dir, out float t[2])
{
    float Epsilon = 0.0000001;
    
    vec3 W = normalize(gCapsuleB - gCapsuleA);
    vec3 U = perp(W);
    vec3 V = cross(U, W);
    U = normalize(cross(V, W));
    V = normalize(V);
    
    vec3 CapsuleA = gCapsuleA - W * 2.0 * Radius;
    vec3 CapsuleB = gCapsuleB + W * 2.0 * Radius;

    float rSqr = Radius*Radius;
    float extent = 0.25 * distance(CapsuleA, CapsuleB);

    vec3 diff = origin - 0.5 * (CapsuleB + CapsuleA);
    mat3 basis = mat3(U, V, W);
    vec3 P = diff * basis;

    float dz = dot(W, dir);
    if (abs(dz) >= 1.0 - Epsilon) {
        float radialSqrDist = rSqr - P.x*P.x - P.y*P.y;
        if (radialSqrDist < 0.0)
            return 0;

        float zOffset = sqrt(radialSqrDist) + extent;
        if (dz > 0.0) {
            t[0] = -P.z - zOffset;
            t[1] = -P.z + zOffset;
        } else {
            t[0] = P.z - zOffset;
            t[1] = P.z + zOffset;
        }
        return 2;
    }

    vec3 D = vec3(dot(U, dir), dot(V, dir), dz);

    float a0 = P.x*P.x + P.y*P.y - rSqr;
    float a1 = P.x*D.x + P.y*D.y;
    float a2 = D.x*D.x + D.y*D.y;
    float discr = a1*a1 - a0*a2;
    if (discr < 0.0)
        return 0;

    float root, inv, tValue, zValue;
    int quantity = 0;
    if (discr > Epsilon) {
        root = sqrt(discr);
        inv = 1.0/a2;
        tValue = (-a1 - root)*inv;
        zValue = P.z + tValue*D.z;
        if (abs(zValue) <= extent)
            t[quantity++] = tValue;

        tValue = (-a1 + root)*inv;
        zValue = P.z + tValue*D.z;
        if (abs(zValue) <= extent)
            t[quantity++] = tValue;

        if (quantity == 2)
            return 2;
    } else {
        tValue = -a1/a2;
        zValue = P.z + tValue*D.z;
        if (abs(zValue) <= extent) {
            t[0] = tValue;
            return 1;
        }
    }

    float PZpE = P.z + extent;
    a1 += PZpE*D.z;
    a0 += PZpE*PZpE;
    discr = a1*a1 - a0;
    if (discr > Epsilon) {
        root = sqrt(discr);
        tValue = -a1 - root;
        zValue = P.z + tValue*D.z;
        if (zValue <= -extent) {
            t[quantity++] = tValue;
            if (quantity == 2) {
                if (t[0] > t[1]) {
                    float save = t[0];
                    t[0] = t[1];
                    t[1] = save;
                }
                return 2;
            }
        }

        tValue = -a1 + root;
        zValue = P.z + tValue*D.z;
        if (zValue <= -extent) {
            t[quantity++] = tValue;
            if (quantity == 2) {
                if (t[0] > t[1]) {
                    float save = t[0];
                    t[0] = t[1];
                    t[1] = save;
                }
                return 2;
            }
        }
    } else if (abs(discr) <= Epsilon) {
        tValue = -a1;
        zValue = P.z + tValue*D.z;
        if (zValue <= -extent) {
            t[quantity++] = tValue;
            if (quantity == 2) {
                if (t[0] > t[1]) {
                    float save = t[0];
                    t[0] = t[1];
                    t[1] = save;
                }
                return 2;
            }
        }
    }

    a1 -= 2.0*extent*D.z;
    a0 -= 4.0*extent*P.z;
    discr = a1*a1 - a0;
    if (discr > Epsilon) {
        root = sqrt(discr);
        tValue = -a1 - root;
        zValue = P.z + tValue*D.z;
        if (zValue >= extent) {
            t[quantity++] = tValue;
            if (quantity == 2) {
                if (t[0] > t[1]) {
                    float save = t[0];
                    t[0] = t[1];
                    t[1] = save;
                }
                return 2;
            }
        }

        tValue = -a1 + root;
        zValue = P.z + tValue*D.z;
        if (zValue >= extent) {
            t[quantity++] = tValue;
            if (quantity == 2) {
                if (t[0] > t[1]) {
                    float save = t[0];
                    t[0] = t[1];
                    t[1] = save;
                }
                return 2;
            }
        }
    } else if (abs(discr) <= Epsilon) {
        tValue = -a1;
        zValue = P.z + tValue*D.z;
        if (zValue >= extent) {
            t[quantity++] = tValue;
            if (quantity == 2) {
                if (t[0] > t[1]) {
                    float save = t[0];
                    t[0] = t[1];
                    t[1] = save;
                }
                return 2;
            }
        }
    }

    return quantity;
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
    vec3 rayStart = vec3(gPosition, gMinZ);
    vec3 rayEnd = vec3(gPosition, gMaxZ);
    vec3 rayDir = normalize(rayEnd - rayStart);
    
    float d[2];
    int hitCount = IntersectCapsule(rayStart, rayDir, d);
    if (hitCount > 0) {
    
        if (hitCount > 1)
            d[0] = d[1];

        vec3 hitPoint = rayStart + d[0] * rayDir;
        vec4 ndc = Projection * vec4(hitPoint, 1);
        vec3 x0 = hitPoint;
        vec3 x1 = gCapsuleA;
        vec3 x2 = gCapsuleB;
/*        
        // Point-Line distance
        float dist = length(cross(x2 - x1, x1 - x0)) / length(x2 - x1);
*/
        float length = distance(x1, x2);
        vec3 v = (x2 - x1) / length;
/*
        x1 = x1 + Radius * v;
        x2 = x2 - Radius * v;
        length -= Radius * 2;
*/
        float t = dot(x0 - x1, v);
        vec3 spinePoint;
        if (t > length) spinePoint = x2;
        else if (t > 0.0) spinePoint = x1 + t * v;
        else spinePoint = x1;

        vec3 N = -normalize(hitPoint - spinePoint);
        vec3 color = AmbientMaterial + ComputeLight(LightDirection, N, true);

        gl_FragDepth = ndc.z / ndc.w;
        gl_FragColor = vec4(color, 1.0);
        
    } else {
        gl_FragDepth = 0.9;
        gl_FragColor = 0.5 * Color;
        return;
    }
}
