#include "m3d/camera.h"

#include <algorithm>

namespace m3d {

namespace {
constexpr Vec3 kUp{0, 0, 1};
constexpr float kMaxPitch = 89.0f * kPi / 180.0f;
}  // namespace

void OrbitCamera::orbit(float dxPixels, float dyPixels) {
    yaw -= dxPixels * kRadiansPerPixel;
    pitch = std::clamp(pitch + dyPixels * kRadiansPerPixel, -kMaxPitch, kMaxPitch);
}

void OrbitCamera::pan(float dxPixels, float dyPixels, float viewportHeightPixels) {
    if (viewportHeightPixels <= 0) return;
    // World units per pixel at the target's depth, so the target follows the fingers.
    float worldPerPixel = 2.0f * distance * std::tan(fovY * 0.5f) / viewportHeightPixels;
    Vec3 forward = normalize(target - eye());
    Vec3 right = normalize(cross(forward, kUp));
    Vec3 up = cross(right, forward);
    target += right * (-dxPixels * worldPerPixel) + up * (dyPixels * worldPerPixel);
}

void OrbitCamera::zoom(float scale) {
    if (scale <= 0) return;
    distance = std::clamp(distance / scale, kMinDistance, kMaxDistance);
}

Vec3 OrbitCamera::eye() const {
    float cp = std::cos(pitch);
    return target + Vec3{cp * std::cos(yaw), cp * std::sin(yaw), std::sin(pitch)} * distance;
}

Mat4 OrbitCamera::view() const { return lookAt(eye(), target, kUp); }

Mat4 OrbitCamera::projection(float aspect) const {
    float zNear = std::max(distance * 0.01f, 0.001f);
    float zFar = distance * 100.0f + 100.0f;
    return perspective(fovY, aspect, zNear, zFar);
}

}  // namespace m3d
