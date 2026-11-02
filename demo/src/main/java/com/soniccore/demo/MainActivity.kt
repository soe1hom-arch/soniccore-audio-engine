/*
 * Copyright 2026 SonicCore contributors
 * SPDX-License-Identifier: Apache-2.0
 */

package com.soniccore.demo

import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.widget.Button
import android.widget.LinearLayout
import android.widget.SeekBar
import android.widget.Switch
import android.widget.TextView
import androidx.activity.ComponentActivity
import androidx.activity.result.contract.ActivityResultContracts
import androidx.media3.common.MediaItem
import androidx.media3.common.audio.AudioProcessor
import androidx.media3.exoplayer.DefaultRenderersFactory
import androidx.media3.exoplayer.ExoPlayer
import androidx.media3.exoplayer.audio.AudioSink
import androidx.media3.exoplayer.audio.DefaultAudioSink
import com.soniccore.android.SonicCoreAudioProcessor

/** Small local-file player for validating SonicCore against real music. */
class MainActivity : ComponentActivity() {
    private val effects = SonicCoreAudioProcessor()
    private lateinit var player: ExoPlayer
    private lateinit var status: TextView

    private val openAudio = registerForActivityResult(
        ActivityResultContracts.OpenDocument()
    ) { uri ->
        if (uri == null) return@registerForActivityResult
        runCatching {
            contentResolver.takePersistableUriPermission(
                uri,
                Intent.FLAG_GRANT_READ_URI_PERMISSION
            )
        }
        player.setMediaItem(MediaItem.fromUri(uri))
        player.prepare()
        status.text = "File siap diputar"
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        player = ExoPlayer.Builder(this, SonicCoreRenderersFactory()).build()

        val root = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            setPadding(32, 32, 32, 32)
        }
        root.addView(TextView(this).apply {
            text = "SonicCore Music Test"
            textSize = 24f
        })
        status = TextView(this).apply { text = "Pilih file audio untuk mulai" }
        root.addView(status)
        root.addView(Button(this).apply {
            text = "Pilih file audio"
            setOnClickListener { openAudio.launch(arrayOf("audio/*")) }
        })
        root.addView(Button(this).apply {
            text = "Play / Pause"
            setOnClickListener {
                if (player.isPlaying) player.pause() else player.play()
                status.text = if (player.isPlaying) "Sedang diputar" else "Dijeda"
            }
        })

        val bypass = Switch(this).apply { text = "Bypass DSP" }
        val spatial = Switch(this).apply { text = "Spatial 3D/8D" }
        val reverb = Switch(this).apply { text = "Room reverb" }
        root.addView(bypass)
        root.addView(spatial)
        root.addView(reverb)
        root.addView(TextView(this).apply { text = "Bass (-6 dB .. +6 dB)" })
        val bass = SeekBar(this).apply { max = 120; progress = 60 }
        root.addView(bass)

        fun updateEffects() {
            effects.bypassed = bypass.isChecked
            effects.spatialEnabled = spatial.isChecked
            effects.spatialMix = if (spatial.isChecked) 1f else 0f
            effects.spatialRotate = spatial.isChecked
            effects.reverbEnabled = reverb.isChecked
            effects.reverbMix = if (reverb.isChecked) 0.2f else 0f
            effects.bassDb = bass.progress / 10f - 6f
            effects.markParametersChanged()
        }
        bypass.setOnCheckedChangeListener { _, _ -> updateEffects() }
        spatial.setOnCheckedChangeListener { _, _ -> updateEffects() }
        reverb.setOnCheckedChangeListener { _, _ -> updateEffects() }
        bass.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(bar: SeekBar?, value: Int, fromUser: Boolean) = updateEffects()
            override fun onStartTrackingTouch(bar: SeekBar?) = Unit
            override fun onStopTrackingTouch(bar: SeekBar?) = Unit
        })
        updateEffects()
        setContentView(root)
    }

    override fun onDestroy() {
        player.release()
        effects.reset()
        super.onDestroy()
    }

    private inner class SonicCoreRenderersFactory : DefaultRenderersFactory(this@MainActivity) {
        override fun buildAudioSink(
            context: android.content.Context,
            enableFloatOutput: Boolean,
            enableAudioTrackPlaybackParams: Boolean
        ): AudioSink = DefaultAudioSink.Builder(context)
            .setEnableFloatOutput(true)
            .setAudioProcessors(arrayOf<AudioProcessor>(effects))
            .setEnableAudioTrackPlaybackParams(enableAudioTrackPlaybackParams)
            .build()
    }
}
