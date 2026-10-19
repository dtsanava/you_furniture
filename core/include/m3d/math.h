// Minimal vector/matrix math for the editor core.
// Conventions: right-handed, Z-up (same as Blender), column-major matrices (OpenGL layout).
#pragma once

#include <cmath>

namespace m3d {

constexpr float kPi = 3.14159265358979323846f;

struct Vec3 {
    float x = 0, y = 0, z = 0;

    Vec3() = default;
    constexpr Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    Vec3 operator-() const { return {-x, -y, -z}; }
    Vec3& operator+=(const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
};

inline float dot(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

inline Vec3 cross(const Vec3& a, const Vec3& b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

inline float length(const Vec3& v) { return std::sqrt(dot(v, v)); }

inline Vec3 normalize(const Vec3& v) {
    float len = length(v);
    return len > 0 ? v * (1.0f / len) : Vec3{};
}

// 4x4 matrix, column-major: m[col * 4 + row].
struct Mat4 {
    float m[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

    static Mat4 identity() { return {}; }

    float& at(int row, int col) { return m[col * 4 + row]; }
    float at(int row, int col) const { return m[col * 4 + row]; }

    Mat4 operator*(const Mat4& o) const {
        Mat4 r;
        for (int c = 0; c < 4; ++c)
            for (int rr = 0; rr < 4; ++rr) {
                float s = 0;
                for (int k = 0; k < 4; ++k) s += at(rr, k) * o.at(k, c);
                r.at(rr, c) = s;
            }
        return r;
    }

    Vec3 transformPoint(const Vec3& p) const {
        float x = at(0, 0) * p.x + at(0, 1) * p.y + at(0, 2) * p.z + at(0, 3);
        float y = at(1, 0) * p.x + at(1, 1) * p.y + at(1, 2) * p.z + at(1, 3);
        float z = at(2, 0) * p.x + at(2, 1) * p.y + at(2, 2) * p.z + at(2, 3);
        float w = at(3, 0) * p.x + at(3, 1) * p.y + at(3, 2) * p.z + at(3, 3);
        return w != 0 ? Vec3{x / w, y / w, z / w} : Vec3{x, y, z};
    }
};

Mat4 perspective(float fovYRadians, float aspect, float zNear, float zFar);
Mat4 lookAt(const Vec3& eye, const Vec3& target, const Vec3& up);

}  // namespace m3d
