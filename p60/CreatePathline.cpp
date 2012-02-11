#include <cmath>
#include "Splat.h"

PointList CreatePathline()
{
    PointList path(64);

    const float dtheta = TwoPi / float(path.size());
    const float r = 0.5f;

    PointList::iterator i = path.begin();
    for (float theta = 0; i != path.end(); ++i, theta += dtheta) {
        i->setX(r * std::cos(theta));
        i->setY(r * std::sin(theta));
        i->setZ(0);
    }

    return path;
}
