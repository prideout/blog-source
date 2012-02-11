#include "Common.hpp"

using namespace vmath;

class Trackball : public ITrackball {
    public:
        Trackball(float width, float height, float radius);
        void MouseDown(int x, int y);
        void MouseUp(int x, int y);
        void MouseMove(int x, int y);
        void ReturnHome();
        vmath::Matrix3 GetRotation() const;
        void Update(unsigned int microseconds);
    private:
        vmath::Vector3 MapToSphere(int x, int y);
        vmath::Vector3 m_startPos;
        vmath::Vector3 m_currentPos;
        vmath::Vector3 m_previousPos;
        vmath::Vector3 m_axis;
        vmath::Quat m_quat;
        bool m_active;
        float m_radius;
        float m_radiansPerSecond;
        float m_width;
        float m_height;
        unsigned int m_currentTime;
        unsigned int m_previousTime;

        struct VoyageHome {
            bool Active;
            vmath::Quat DepartureQuat;
            unsigned int microseconds;
        } m_voyageHome;

        struct Inertia {
            bool Active;
            vmath::Vector3 Axis;
            float RadiansPerSecond;
        } m_inertia;
};

Trackball::Trackball(float width, float height, float radius)
{
    m_currentTime = 0;
    m_inertia.Active = false;
    m_voyageHome.Active = false;
    m_active = false;
    m_quat = vmath::Quat::identity();
    m_radius = radius;
    m_startPos = m_currentPos = m_previousPos = Vector3(0);
    m_width = width;
    m_height = height;
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
    unsigned int microseconds = m_currentTime - m_previousTime;
    if (radians > 0.01f && microseconds > 0) {
        m_radiansPerSecond = 1000000.0f * radians / microseconds;
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
    x = int(m_width) - x;
    const float SafeRadius = m_radius * 0.99f;
    float fx = x - m_width / 2.0f;
    float fy = y - m_height / 2.0f;

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

void Trackball::Update(unsigned int microseconds)
{
    m_currentTime += microseconds;

    if (m_voyageHome.Active) {
        m_voyageHome.microseconds += microseconds;
        float t = m_voyageHome.microseconds / 200000.0f;
        
        if (t > 1) {
            m_quat = Quat::identity();
            m_voyageHome.Active = false;
            return;
        }

        m_quat = slerp(t, m_voyageHome.DepartureQuat, Quat::identity());
        m_inertia.Active = false;
    }

    if (m_inertia.Active) {
        m_inertia.RadiansPerSecond -= 0.00001f * microseconds;
        if (m_inertia.RadiansPerSecond < 0) {
            m_inertia.Active = false;
            m_radiansPerSecond = 0;
        } else {
            Quat q = Quat::rotation(m_inertia.RadiansPerSecond * microseconds * 0.000001f, m_inertia.Axis);
            m_quat = rotate(q, m_quat);
        }
    }
}

void Trackball::ReturnHome()
{
    m_voyageHome.Active = true;
    m_voyageHome.DepartureQuat = m_quat;
    m_voyageHome.microseconds = 0;
}

ITrackball* CreateTrackball(float width, float height, float radius)
{
    return new Trackball(width, height, radius);
}
