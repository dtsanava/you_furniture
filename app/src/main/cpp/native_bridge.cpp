// JNI entry points for com.m3d.editor.NativeBridge.
// Every function runs on the GLSurfaceView render thread (gestures are posted with queueEvent),
// so the editor state needs no locking.
#include <jni.h>

#include <memory>

#include "m3d/camera.h"
#include "m3d/mesh.h"
#include "renderer.h"

namespace {

struct Editor {
    m3d::OrbitCamera camera;
    m3d::Mesh mesh = m3d::makeCube(2.0f);
    m3d::Renderer renderer;
};

std::unique_ptr<Editor> g_editor;

Editor& editor() {
    if (!g_editor) g_editor = std::make_unique<Editor>();
    return *g_editor;
}

}  // namespace

extern "C" {

JNIEXPORT jboolean JNICALL Java_com_m3d_editor_NativeBridge_onSurfaceCreated(JNIEnv*, jclass) {
    return editor().renderer.init() ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL Java_com_m3d_editor_NativeBridge_onSurfaceChanged(JNIEnv*, jclass, jint width,
                                                                         jint height) {
    editor().renderer.resize(width, height);
}

JNIEXPORT void JNICALL Java_com_m3d_editor_NativeBridge_onDrawFrame(JNIEnv*, jclass) {
    Editor& e = editor();
    e.renderer.draw(e.camera, e.mesh);
}

JNIEXPORT void JNICALL Java_com_m3d_editor_NativeBridge_orbit(JNIEnv*, jclass, jfloat dx, jfloat dy) {
    editor().camera.orbit(dx, dy);
}

JNIEXPORT void JNICALL Java_com_m3d_editor_NativeBridge_pan(JNIEnv*, jclass, jfloat dx, jfloat dy) {
    Editor& e = editor();
    e.camera.pan(dx, dy, static_cast<float>(e.renderer.height()));
}

JNIEXPORT void JNICALL Java_com_m3d_editor_NativeBridge_zoom(JNIEnv*, jclass, jfloat scale) {
    editor().camera.zoom(scale);
}

}  // extern "C"
