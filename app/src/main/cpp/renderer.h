// OpenGL ES 3.0 viewport renderer. Owns GPU resources; all calls must happen on the GL thread.
#pragma once

#include <GLES3/gl3.h>

#include "m3d/camera.h"
#include "m3d/mesh.h"

namespace m3d {

class Renderer {
public:
    // Creates GPU resources. Call again after the GL context is recreated.
    bool init();
    void resize(int width, int height);
    void draw(const OrbitCamera& camera, const Mesh& mesh);
    void release();

    int width() const { return width_; }
    int height() const { return height_; }

private:
    void uploadMesh(const Mesh& mesh);

    GLuint litProgram_ = 0;
    GLuint lineProgram_ = 0;
    GLuint meshVao_ = 0, meshVbo_ = 0;
    GLuint edgeVao_ = 0, edgeVbo_ = 0;
    GLuint gridVao_ = 0, gridVbo_ = 0;
    GLsizei meshVertexCount_ = 0;
    GLsizei edgeVertexCount_ = 0;
    GLsizei gridVertexCount_ = 0;
    const Mesh* uploadedMesh_ = nullptr;
    int width_ = 1, height_ = 1;
};

}  // namespace m3d
