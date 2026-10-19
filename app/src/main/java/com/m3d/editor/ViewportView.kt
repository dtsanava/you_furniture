package com.m3d.editor

import android.content.Context
import android.opengl.GLSurfaceView
import android.util.Log
import android.view.MotionEvent
import android.view.ScaleGestureDetector
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

/**
 * 3D viewport. Gestures: one finger orbits, two fingers pan, pinch zooms.
 * Gesture deltas are posted to the render thread, where the native editor lives.
 */
class ViewportView(context: Context) : GLSurfaceView(context) {

    private var lastX = 0f
    private var lastY = 0f
    private var lastPointerCount = 0

    private val scaleDetector = ScaleGestureDetector(context,
        object : ScaleGestureDetector.SimpleOnScaleGestureListener() {
            override fun onScale(detector: ScaleGestureDetector): Boolean {
                val scale = detector.scaleFactor
                queueEvent { NativeBridge.zoom(scale) }
                requestRender()
                return true
            }
        })

    init {
        setEGLContextClientVersion(3)
        setEGLConfigChooser(8, 8, 8, 8, 24, 0)
        preserveEGLContextOnPause = true
        setRenderer(object : Renderer {
            override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
                if (!NativeBridge.onSurfaceCreated()) Log.e(TAG, "Native renderer failed to initialize")
            }

            override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
                NativeBridge.onSurfaceChanged(width, height)
            }

            override fun onDrawFrame(gl: GL10?) {
                NativeBridge.onDrawFrame()
            }
        })
        renderMode = RENDERMODE_WHEN_DIRTY
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {
        scaleDetector.onTouchEvent(event)

        // Track the centroid of all fingers so adding or lifting a finger does not make the view jump.
        var cx = 0f
        var cy = 0f
        val count = event.pointerCount
        val skipIndex = if (event.actionMasked == MotionEvent.ACTION_POINTER_UP) event.actionIndex else -1
        var used = 0
        for (i in 0 until count) {
            if (i == skipIndex) continue
            cx += event.getX(i)
            cy += event.getY(i)
            used++
        }
        if (used == 0) return true
        cx /= used
        cy /= used

        when (event.actionMasked) {
            MotionEvent.ACTION_MOVE -> {
                if (used == lastPointerCount) {
                    val dx = cx - lastX
                    val dy = cy - lastY
                    if (used == 1) {
                        queueEvent { NativeBridge.orbit(dx, dy) }
                    } else {
                        queueEvent { NativeBridge.pan(dx, dy) }
                    }
                    requestRender()
                }
            }
        }
        lastX = cx
        lastY = cy
        lastPointerCount = if (event.actionMasked == MotionEvent.ACTION_UP ||
            event.actionMasked == MotionEvent.ACTION_CANCEL) 0 else used
        return true
    }

    private companion object {
        const val TAG = "m3d"
    }
}
