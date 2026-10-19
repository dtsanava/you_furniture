// Polygon mesh and the GPU-ready buffers built from it.
// This is the skeleton's simple face-list mesh; the modeling stage replaces it with a half-edge mesh.
#pragma once

#include <cstdint>
#include <vector>

#include "m3d/math.h"

namespace m3d {

struct Mesh {
    std::vector<Vec3> positions;
    // Each face is a list of vertex indices, counter-clockwise when seen from outside (n-gons allowed).
    std::vector<std::vector<uint32_t>> faces;
};

Mesh makeCube(float size = 2.0f);

// Interleaved triangle vertices for flat shading: x y z nx ny nz per vertex.
std::vector<float> buildFlatTriangles(const Mesh& mesh);

// Unique edges as line vertices: x y z per vertex, two vertices per edge.
std::vector<float> buildEdgeLines(const Mesh& mesh);

// Floor grid on the XY plane as colored lines: x y z r g b per vertex.
// The X axis is drawn red and the Y axis green, as in Blender.
std::vector<float> buildGridLines(int halfCount, float step);

}  // namespace m3d
