// Orbit camera driven by touch gestures: rotate around a target, pan, zoom.
#pragma once

#include "m3d/math.h"

namespace m3d {

class OrbitCamera {
public:
    // Rotate by a drag of (dx, dy) pixels.
    void orbit(float dxPixels, float dyPixels);
    // Move the target in the view plane by a drag of (dx, dy) pixels.
    void pan(float dxPixels, float dyPixels, float viewportHeightPixels);
    // Multiply the distance to the target; scale > 1 moves closer (pinch out).
    void zoom(float scale);

    Vec3 eye() const;
    Mat4 view() const;
    Mat4 projection(float aspect) const;

    Vec3 target{0, 0, 0};
    float yaw = 45.0f * kPi / 180.0f;    // around Z
    float pitch = 30.0f * kPi / 180.0f;  // above the XY plane
    float distance = 8.0f;
    float fovY = 50.0f * kPi / 180.0f;

    static constexpr float kRadiansPerPixel = 0.008f;
    static constexpr float kMinDistance = 0.05f;
    static constexpr float kMaxDistance = 1000.0f;
};

}  // namespace m3d
