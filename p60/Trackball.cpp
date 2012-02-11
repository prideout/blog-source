#include "Pez.h"
#include "Trackball.h"

using namespace VectorMath;

Trackball::Trackball(float radius)
{
    m_currentTime = 0;
    m_inertia.Active = false;
    m_voyageHome.Active = false;
    m_active = false;
    m_quat = VectorMath::Quat::identity();
    m_radius = radius;
    m_startPos = m_currentPos = m_previousPos = Vector3(0);
}

void Trackball::MouseDown(int x, int y)
{
    m_radiansPerSecond = 0;
    m_previousPos = m_currentPos = m_startPos = MapToSphere(x, y);
    m_active = true;
}

void Trackball::MouseUp(int x, int y)
{
    m_active = false;

    Quat q = Quat::rotation(m_startPos, m_currentPos);
    m_quat = rotate(q, m_quat);

    if (m_radiansPerSecond > 0) {
        m_inertia.Active = true;
        m_inertia.RadiansPerSecond = m_radiansPerSecond;
        m_inertia.Axis = m_axis;
    }
}

void Trackball::MouseMove(int x, int y)
{
    m_currentPos = MapToSphere(x, y);

    float radians = acos(dot(m_previousPos, m_currentPos));
    unsigned int elapsedMicroseconds = m_currentTime - m_previousTime;
    if (radians > 0.01f && elapsedMicroseconds > 0) {
        m_radiansPerSecond = 1000000.0f * radians / elapsedMicroseconds;
        m_axis = normalize(cross(m_previousPos, m_currentPos));
    } else {
        m_radiansPerSecond = 0;
    }

    m_previousPos = m_currentPos;
    m_previousTime = m_currentTime;
}

Matrix3 Trackball::GetRotation() const
{
    if (!m_active)
        return Matrix3(m_quat);

    Quat q = Quat::rotation(m_startPos, m_currentPos);
    return Matrix3(rotate(q, m_quat));
}

Vector3 Trackball::MapToSphere(int x, int y)
{
    x = PEZ_VIEWPORT_WIDTH - x;
    const float SafeRadius = m_radius - 1;
    float fx = (float) x - PEZ_VIEWPORT_WIDTH / 2;
    float fy = (float) y - PEZ_VIEWPORT_HEIGHT / 2;

    float lenSqr = fx*fx+fy*fy;
    
    if (lenSqr > SafeRadius*SafeRadius) {
        float theta = atan2(fy, fx);
        fx = SafeRadius * cos(theta);
        fy = SafeRadius * sin(theta);
    }
    
    lenSqr = fx*fx+fy*fy;
    float z = sqrt(m_radius*m_radius - lenSqr);
    return Vector3(fx, fy, z) / m_radius;
}

void Trackball::Update(unsigned int elapsedMicroseconds)
{
    m_currentTime += elapsedMicroseconds;

    if (m_voyageHome.Active) {
        m_voyageHome.ElapsedMicroseconds += elapsedMicroseconds;
        float t = m_voyageHome.ElapsedMicroseconds / 200000.0f;
        
        if (t > 1) {
            m_quat = Quat::identity();
            m_voyageHome.Active = false;
            return;
        }

        m_quat = slerp(t, m_voyageHome.DepartureQuat, Quat::identity());
        m_inertia.Active = false;
    }

    if (m_inertia.Active) {
        m_inertia.RadiansPerSecond -= 0.00001f * elapsedMicroseconds;
        if (m_inertia.RadiansPerSecond < 0) {
            m_inertia.Active = false;
            m_radiansPerSecond = 0;
        } else {
            Quat q = Quat::rotation(m_inertia.RadiansPerSecond * elapsedMicroseconds * 0.000001f, m_inertia.Axis);
            m_quat = rotate(q, m_quat);
        }
    }
}

void Trackball::ReturnHome()
{
    m_voyageHome.Active = true;
    m_voyageHome.DepartureQuat = m_quat;
    m_voyageHome.ElapsedMicroseconds = 0;
}
