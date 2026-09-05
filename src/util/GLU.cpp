#include "util/GLU.h"

#include <cmath>

void gluPerspective(float fovy, float aspect, float zNear, float zFar)
{
    
    double const height = zNear * tanf(static_cast<float>(fovy * 3.14159265358979323846 / 360.0));
    double const width = height * aspect;
    glFrustum(-width, width, -height, height, zNear, zFar);
}
