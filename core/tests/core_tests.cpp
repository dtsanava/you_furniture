// Core tests. A tiny self-contained runner keeps the build free of external dependencies.
#include <cmath>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

#include "m3d/camera.h"
#include "m3d/math.h"
#include "m3d/mesh.h"

using namespace m3d;

namespace {

struct TestCase {
    const char* name;
    std::function<void()> fn;
};

std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

int g_failures = 0;

struct Register {
    Register(const char* name, std::function<void()> fn) { registry().push_back({name, std::move(fn)}); }
};

#define TEST(name)                                  \
    void name();                                    \
    Register reg_##name(#name, name);               \
    void name()

#define EXPECT(cond)                                                          \
    do {                                                                      \
        if (!(cond)) {                                                        \
            std::printf("  FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);     \
            ++g_failures;                                                     \
        }                                                                     \
    } while (0)

bool near(float a, float b, float eps = 1e-4f) { return std::fabs(a - b) <= eps; }
bool near(const Vec3& a, const Vec3& b, float eps = 1e-4f) {
    return near(a.x, b.x, eps) && near(a.y, b.y, eps) && near(a.z, b.z, eps);
}

TEST(lookAtMapsTargetOntoNegativeZ) {
    Mat4 v = lookAt({0, -5, 0}, {0, 0, 0}, {0, 0, 1});
    EXPECT(near(v.transformPoint({0, 0, 0}), {0, 0, -5}));
    // +X in the world is to the right of a camera looking along +Y.
    EXPECT(v.transformPoint({1, 0, 0}).x > 0);
    // +Z (up) stays up on screen.
    EXPECT(v.transformPoint({0, 0, 1}).y > 0);
}

TEST(perspectiveMapsNearAndFarToClipRange) {
    Mat4 p = perspective(kPi / 2, 1.0f, 1.0f, 10.0f);
    EXPECT(near(p.transformPoint({0, 0, -1}).z, -1.0f));
    EXPECT(near(p.transformPoint({0, 0, -10}).z, 1.0f));
}

TEST(cubeHasSixOutwardFaces) {
    Mesh cube = makeCube(2.0f);
    EXPECT(cube.positions.size() == 8);
    EXPECT(cube.faces.size() == 6);
    std::vector<float> tris = buildFlatTriangles(cube);
    EXPECT(tris.size() == 6 * 2 * 3 * 6);  // faces * triangles * vertices * floats
    for (size_t v = 0; v < tris.size(); v += 6) {
        Vec3 p{tris[v], tris[v + 1], tris[v + 2]};
        Vec3 n{tris[v + 3], tris[v + 4], tris[v + 5]};
        EXPECT(near(length(n), 1.0f));
        EXPECT(dot(p, n) > 0);  // normal points away from the cube's center
    }
}

TEST(cubeHasTwelveUniqueEdges) {
    EXPECT(buildEdgeLines(makeCube()).size() == 12 * 2 * 3);
}

TEST(gridHasAxisColoredCenterLines) {
    std::vector<float> grid = buildGridLines(10, 1.0f);
    EXPECT(grid.size() == 21 * 2 * 2 * 6);
    bool sawRed = false, sawGreen = false;
    for (size_t v = 0; v < grid.size(); v += 6) {
        if (grid[v + 3] > 0.8f) sawRed = true;
        if (grid[v + 4] > 0.6f && grid[v + 3] < 0.5f) sawGreen = true;
    }
    EXPECT(sawRed && sawGreen);
}

TEST(orbitKeepsDistanceAndClampsPitch) {
    OrbitCamera cam;
    cam.orbit(300, 0);
    EXPECT(near(length(cam.eye() - cam.target), cam.distance));
    cam.orbit(0, 100000);
    EXPECT(cam.pitch < kPi / 2);
    cam.orbit(0, -200000);
    EXPECT(cam.pitch > -kPi / 2);
}

TEST(zoomIsClamped) {
    OrbitCamera cam;
    cam.zoom(2.0f);
    EXPECT(near(cam.distance, 4.0f));
    cam.zoom(1e9f);
    EXPECT(near(cam.distance, OrbitCamera::kMinDistance));
    cam.zoom(1e-9f);
    EXPECT(near(cam.distance, OrbitCamera::kMaxDistance));
    cam.zoom(0.0f);  // ignored
    EXPECT(near(cam.distance, OrbitCamera::kMaxDistance));
}

TEST(panMovesTargetWithFingers) {
    OrbitCamera cam;
    cam.yaw = -kPi / 2;  // camera on -Y looking along +Y, +X to the right
    cam.pitch = 0;
    cam.pan(-100, 0, 1000);  // fingers move left: scene follows, target moves right
    EXPECT(cam.target.x > 0);
    EXPECT(near(cam.target.z, 0));
    Vec3 before = cam.target;
    cam.pan(0, 100, 1000);  // fingers move down: target moves up
    EXPECT(cam.target.z > before.z);
}

}  // namespace

int main() {
    for (const auto& t : registry()) {
        int before = g_failures;
        t.fn();
        std::printf("%s %s\n", g_failures == before ? "ok  " : "FAIL", t.name);
    }
    std::printf("%zu tests, %d failures\n", registry().size(), g_failures);
    return g_failures == 0 ? 0 : 1;
}
