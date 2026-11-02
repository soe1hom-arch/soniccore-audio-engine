/*
 * Copyright 2026 SonicCore contributors
 * SPDX-License-Identifier: Apache-2.0
 */

package com.soniccore.demo

import android.app.Activity
import android.media.AudioAttributes
import android.media.AudioFormat
import android.media.AudioTrack
import android.os.Bundle
import android.widget.Button
import android.widget.LinearLayout
import android.widget.SeekBar
import android.widget.Switch
import android.widget.TextView
import com.soniccore.android.SonicCoreNative
import java.util.concurrent.atomic.AtomicBoolean
import kotlin.math.PI
import kotlin.math.sin

/** Small hardware test surface: it deliberately plays a deterministic tone. */
class MainActivity : Activity() {
    private val running = AtomicBoolean(false)
    private var worker: Thread? = null
    private var engine: SonicCoreNative? = null
    private var track: AudioTrack? = null
    private var bypass = false
    private var spatial = false
    private var reverb = false
    private var bassDb = 0f
    private var playButton: Button? = null
    private val flatEq = FloatArray(10)

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val root = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            setPadding(32, 32, 32, 32)
        }
        root.addView(TextView(this).apply {
            text = "SonicCore hardware test\n440 Hz stereo tone"
            textSize = 22f
        })
        val play = Button(this).apply { text = "Mulai test tone" }
        playButton = play
        val bypassSwitch = Switch(this).apply { text = "Bypass DSP" }
        val spatialSwitch = Switch(this).apply { text = "Spatial 3D/8D" }
        val reverbSwitch = Switch(this).apply { text = "Room reverb" }
        val bass = SeekBar(this).apply { max = 120; progress = 60 }
        root.addView(play)
        root.addView(bypassSwitch)
        root.addView(spatialSwitch)
        root.addView(reverbSwitch)
        root.addView(TextView(this).apply { text = "Bass (-6 dB .. +6 dB)" })
        root.addView(bass)
        setContentView(root)

        play.setOnClickListener {
            if (running.get()) stopTone(play) else startTone(play)
        }
        bypassSwitch.setOnCheckedChangeListener { _, checked -> bypass = checked }
        spatialSwitch.setOnCheckedChangeListener { _, checked -> spatial = checked }
        reverbSwitch.setOnCheckedChangeListener { _, checked -> reverb = checked }
        bass.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(bar: SeekBar?, value: Int, fromUser: Boolean) {
                bassDb = value / 10f - 6f
            }
            override fun onStartTrackingTouch(bar: SeekBar?) = Unit
            override fun onStopTrackingTouch(bar: SeekBar?) = Unit
        })
    }

    private fun startTone(button: Button) {
        val sampleRate = 48_000
        val channels = 2
        val minBuffer = AudioTrack.getMinBufferSize(
            sampleRate,
            AudioFormat.CHANNEL_OUT_STEREO,
            AudioFormat.ENCODING_PCM_FLOAT
        )
        val bufferBytes = (minBuffer.coerceAtLeast(2048) * 2)
        track = AudioTrack.Builder()
            .setAudioAttributes(
                AudioAttributes.Builder()
                    .setUsage(AudioAttributes.USAGE_MEDIA)
                    .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
                    .build()
            )
            .setAudioFormat(
                AudioFormat.Builder()
                    .setSampleRate(sampleRate)
                    .setEncoding(AudioFormat.ENCODING_PCM_FLOAT)
                    .setChannelMask(AudioFormat.CHANNEL_OUT_STEREO)
                    .build()
            )
            .setBufferSizeInBytes(bufferBytes)
            .setTransferMode(AudioTrack.MODE_STREAM)
            .build()
        engine = SonicCoreNative(sampleRate, channels)
        running.set(true)
        track?.play()
        button.text = "Hentikan test tone"
        worker = Thread {
            val frames = 256
            val samples = FloatArray(frames * channels)
            var phase = 0.0
            while (running.get()) {
                val currentEngine = engine ?: break
                for (frame in 0 until frames) {
                    val value = (sin(phase) * 0.2).toFloat()
                    samples[frame * 2] = value
                    samples[frame * 2 + 1] = value
                    phase += 2.0 * PI * 440.0 / sampleRate
                    if (phase >= 2.0 * PI) phase -= 2.0 * PI
                }
                currentEngine.setBypassed(bypass)
                currentEngine.setParameters(
                    eqGainsDb = flatEq,
                    bassDb = bassDb,
                    spatialMix = if (spatial) 1f else 0f,
                    spatialEnabled = spatial,
                    spatialRotate = spatial,
                    reverbMix = if (reverb) 0.2f else 0f,
                    reverbEnabled = reverb
                )
                currentEngine.process(samples, frames)
                track?.write(samples, 0, samples.size, AudioTrack.WRITE_BLOCKING)
            }
        }.also { it.start() }
    }

    private fun stopTone(button: Button? = playButton) {
        running.set(false)
        worker?.join(300)
        worker = null
        track?.stop()
        track?.release()
        track = null
        engine?.close()
        engine = null
        button?.text = "Mulai test tone"
    }

    override fun onDestroy() {
        if (running.get()) stopTone()
        super.onDestroy()
    }
}
