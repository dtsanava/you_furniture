#include "renderer.h"

#include <android/log.h>

#include <vector>

#define LOG_TAG "m3d"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace m3d {

namespace {

// Flat-shaded surface with a headlight so faces stay readable from any angle.
const char* kLitVs = R"(#version 300 es
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
uniform mat4 uView;
uniform mat4 uProj;
out vec3 vNormalView;
void main() {
    vNormalView = mat3(uView) * aNormal;
    gl_Position = uProj * uView * vec4(aPos, 1.0);
}
)";

const char* kLitFs = R"(#version 300 es
precision mediump float;
in vec3 vNormalView;
uniform vec3 uColor;
out vec4 fragColor;
void main() {
    float light = max(dot(normalize(vNormalView), vec3(0.0, 0.0, 1.0)), 0.0);
    fragColor = vec4(uColor * (0.35 + 0.65 * light), 1.0);
}
)";

// Lines with either per-vertex color (grid) or a uniform color (edges).
const char* kLineVs = R"(#version 300 es
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
uniform mat4 uView;
uniform mat4 uProj;
uniform float uDepthBias;
out vec3 vColor;
void main() {
    vColor = aColor;
    vec4 p = uProj * uView * vec4(aPos, 1.0);
    p.z -= uDepthBias * p.w;
    gl_Position = p;
}
)";

const char* kLineFs = R"(#version 300 es
precision mediump float;
in vec3 vColor;
out vec4 fragColor;
void main() { fragColor = vec4(vColor, 1.0); }
)";

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        LOGE("shader compile failed: %s", log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint linkProgram(const char* vs, const char* fs) {
    GLuint v = compileShader(GL_VERTEX_SHADER, vs);
    GLuint f = compileShader(GL_FRAGMENT_SHADER, fs);
    if (!v || !f) return 0;
    GLuint program = glCreateProgram();
    glAttachShader(program, v);
    glAttachShader(program, f);
    glLinkProgram(program);
    glDeleteShader(v);
    glDeleteShader(f);
    GLint ok = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        LOGE("program link failed: %s", log);
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

// Uploads interleaved float data; attributes are vec3 at location 0 and, if stride is 6, vec3 at location 1.
void uploadInterleaved(GLuint& vao, GLuint& vbo, const std::vector<float>& data, int floatsPerVertex) {
    if (!vao) glGenVertexArrays(1, &vao);
    if (!vbo) glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
    GLsizei stride = floatsPerVertex * sizeof(float);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
    if (floatsPerVertex == 6) {
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(3 * sizeof(float)));
    } else {
        glDisableVertexAttribArray(1);
    }
    glBindVertexArray(0);
}

void setCameraUniforms(GLuint program, const Mat4& view, const Mat4& proj) {
    glUniformMatrix4fv(glGetUniformLocation(program, "uView"), 1, GL_FALSE, view.m);
    glUniformMatrix4fv(glGetUniformLocation(program, "uProj"), 1, GL_FALSE, proj.m);
}

}  // namespace

bool Renderer::init() {
    // A new GL context invalidates every old handle, so forget them instead of deleting.
    *this = Renderer{};
    litProgram_ = linkProgram(kLitVs, kLitFs);
    lineProgram_ = linkProgram(kLineVs, kLineFs);
    if (!litProgram_ || !lineProgram_) return false;

    std::vector<float> grid = buildGridLines(10, 1.0f);
    uploadInterleaved(gridVao_, gridVbo_, grid, 6);
    gridVertexCount_ = static_cast<GLsizei>(grid.size() / 6);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.23f, 0.23f, 0.23f, 1.0f);  // Blender's viewport gray
    return true;
}

void Renderer::resize(int width, int height) {
    width_ = width > 0 ? width : 1;
    height_ = height > 0 ? height : 1;
    glViewport(0, 0, width_, height_);
}

void Renderer::uploadMesh(const Mesh& mesh) {
    std::vector<float> tris = buildFlatTriangles(mesh);
    uploadInterleaved(meshVao_, meshVbo_, tris, 6);
    meshVertexCount_ = static_cast<GLsizei>(tris.size() / 6);

    std::vector<float> edges = buildEdgeLines(mesh);
    uploadInterleaved(edgeVao_, edgeVbo_, edges, 3);
    edgeVertexCount_ = static_cast<GLsizei>(edges.size() / 3);
    uploadedMesh_ = &mesh;
}

void Renderer::draw(const OrbitCamera& camera, const Mesh& mesh) {
    if (!litProgram_) return;
    if (uploadedMesh_ != &mesh) uploadMesh(mesh);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Mat4 view = camera.view();
    Mat4 proj = camera.projection(static_cast<float>(width_) / height_);

    glUseProgram(lineProgram_);
    setCameraUniforms(lineProgram_, view, proj);
    glUniform1f(glGetUniformLocation(lineProgram_, "uDepthBias"), 0.0f);
    glBindVertexArray(gridVao_);
    glDrawArrays(GL_LINES, 0, gridVertexCount_);

    glUseProgram(litProgram_);
    setCameraUniforms(litProgram_, view, proj);
    glUniform3f(glGetUniformLocation(litProgram_, "uColor"), 0.8f, 0.8f, 0.8f);
    glBindVertexArray(meshVao_);
    glDrawArrays(GL_TRIANGLES, 0, meshVertexCount_);

    // Edges drawn slightly toward the camera so they are not hidden by their own faces.
    glUseProgram(lineProgram_);
    glUniform1f(glGetUniformLocation(lineProgram_, "uDepthBias"), 0.0005f);
    glBindVertexArray(edgeVao_);
    glVertexAttrib3f(1, 0.05f, 0.05f, 0.05f);
    glDrawArrays(GL_LINES, 0, edgeVertexCount_);
    glBindVertexArray(0);
}

void Renderer::release() {
    if (litProgram_) glDeleteProgram(litProgram_);
    if (lineProgram_) glDeleteProgram(lineProgram_);
    GLuint vaos[] = {meshVao_, edgeVao_, gridVao_};
    GLuint vbos[] = {meshVbo_, edgeVbo_, gridVbo_};
    glDeleteVertexArrays(3, vaos);
    glDeleteBuffers(3, vbos);
    *this = Renderer{};
}

}  // namespace m3d
