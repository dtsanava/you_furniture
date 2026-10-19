package com.m3d.editor

/** Calls into the C++ editor core. Every call must be made on the GL render thread. */
object NativeBridge {
    init {
        System.loadLibrary("m3d")
    }

    @JvmStatic external fun onSurfaceCreated(): Boolean
    @JvmStatic external fun onSurfaceChanged(width: Int, height: Int)
    @JvmStatic external fun onDrawFrame()

    @JvmStatic external fun orbit(dx: Float, dy: Float)
    @JvmStatic external fun pan(dx: Float, dy: Float)
    @JvmStatic external fun zoom(scale: Float)
}
