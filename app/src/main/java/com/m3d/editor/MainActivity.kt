package com.m3d.editor

import android.app.Activity
import android.os.Bundle

class MainActivity : Activity() {

    private lateinit var viewport: ViewportView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        viewport = ViewportView(this)
        setContentView(viewport)
    }

    override fun onResume() {
        super.onResume()
        viewport.onResume()
    }

    override fun onPause() {
        viewport.onPause()
        super.onPause()
    }
}
