#include "m3d/mesh.h"

#include <algorithm>
#include <set>
#include <utility>

namespace m3d {

Mesh makeCube(float size) {
    float h = size * 0.5f;
    Mesh m;
    m.positions = {
        {-h, -h, -h}, {h, -h, -h}, {h, h, -h}, {-h, h, -h},
        {-h, -h, h},  {h, -h, h},  {h, h, h},  {-h, h, h},
    };
    m.faces = {
        {0, 3, 2, 1},  // bottom (-Z)
        {4, 5, 6, 7},  // top (+Z)
        {0, 1, 5, 4},  // front (-Y)
        {2, 3, 7, 6},  // back (+Y)
        {1, 2, 6, 5},  // right (+X)
        {3, 0, 4, 7},  // left (-X)
    };
    return m;
}

namespace {

// Newell's method: robust normal for any planar or near-planar polygon.
Vec3 faceNormal(const Mesh& mesh, const std::vector<uint32_t>& face) {
    Vec3 n;
    for (size_t i = 0; i < face.size(); ++i) {
        const Vec3& a = mesh.positions[face[i]];
        const Vec3& b = mesh.positions[face[(i + 1) % face.size()]];
        n.x += (a.y - b.y) * (a.z + b.z);
        n.y += (a.z - b.z) * (a.x + b.x);
        n.z += (a.x - b.x) * (a.y + b.y);
    }
    return normalize(n);
}

void pushVertex(std::vector<float>& out, const Vec3& p, const Vec3& n) {
    out.insert(out.end(), {p.x, p.y, p.z, n.x, n.y, n.z});
}

}  // namespace

std::vector<float> buildFlatTriangles(const Mesh& mesh) {
    std::vector<float> out;
    for (const auto& face : mesh.faces) {
        if (face.size() < 3) continue;
        Vec3 n = faceNormal(mesh, face);
        // Fan triangulation is enough for the convex faces of primitives.
        for (size_t i = 1; i + 1 < face.size(); ++i) {
            pushVertex(out, mesh.positions[face[0]], n);
            pushVertex(out, mesh.positions[face[i]], n);
            pushVertex(out, mesh.positions[face[i + 1]], n);
        }
    }
    return out;
}

std::vector<float> buildEdgeLines(const Mesh& mesh) {
    std::set<std::pair<uint32_t, uint32_t>> edges;
    for (const auto& face : mesh.faces)
        for (size_t i = 0; i < face.size(); ++i) {
            uint32_t a = face[i], b = face[(i + 1) % face.size()];
            edges.insert({std::min(a, b), std::max(a, b)});
        }
    std::vector<float> out;
    for (const auto& [a, b] : edges) {
        const Vec3& pa = mesh.positions[a];
        const Vec3& pb = mesh.positions[b];
        out.insert(out.end(), {pa.x, pa.y, pa.z, pb.x, pb.y, pb.z});
    }
    return out;
}

std::vector<float> buildGridLines(int halfCount, float step) {
    const Vec3 kGray{0.33f, 0.33f, 0.33f};
    const Vec3 kRed{0.85f, 0.25f, 0.3f};
    const Vec3 kGreen{0.45f, 0.7f, 0.15f};
    float extent = halfCount * step;
    std::vector<float> out;
    auto line = [&](Vec3 a, Vec3 b, Vec3 c) {
        out.insert(out.end(), {a.x, a.y, a.z, c.x, c.y, c.z, b.x, b.y, b.z, c.x, c.y, c.z});
    };
    for (int i = -halfCount; i <= halfCount; ++i) {
        float t = i * step;
        line({-extent, t, 0}, {extent, t, 0}, i == 0 ? kRed : kGray);    // parallel to X
        line({t, -extent, 0}, {t, extent, 0}, i == 0 ? kGreen : kGray);  // parallel to Y
    }
    return out;
}

}  // namespace m3d
