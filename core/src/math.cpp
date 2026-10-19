#include "m3d/math.h"

namespace m3d {

Mat4 perspective(float fovYRadians, float aspect, float zNear, float zFar) {
    Mat4 r;
    float f = 1.0f / std::tan(fovYRadians * 0.5f);
    for (float& v : r.m) v = 0;
    r.at(0, 0) = f / aspect;
    r.at(1, 1) = f;
    r.at(2, 2) = (zFar + zNear) / (zNear - zFar);
    r.at(2, 3) = (2.0f * zFar * zNear) / (zNear - zFar);
    r.at(3, 2) = -1.0f;
    return r;
}

Mat4 lookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
    Vec3 f = normalize(target - eye);
    Vec3 s = normalize(cross(f, up));
    Vec3 u = cross(s, f);
    Mat4 r;
    r.at(0, 0) = s.x; r.at(0, 1) = s.y; r.at(0, 2) = s.z; r.at(0, 3) = -dot(s, eye);
    r.at(1, 0) = u.x; r.at(1, 1) = u.y; r.at(1, 2) = u.z; r.at(1, 3) = -dot(u, eye);
    r.at(2, 0) = -f.x; r.at(2, 1) = -f.y; r.at(2, 2) = -f.z; r.at(2, 3) = dot(f, eye);
    return r;
}

}  // namespace m3d
