/*
 * Copyright 2026 SonicCore contributors
 * SPDX-License-Identifier: Apache-2.0
 */

package com.soniccore.android

import androidx.media3.common.C
import androidx.media3.common.audio.AudioProcessor
import java.nio.ByteBuffer
import java.nio.ByteOrder

/**
 * Media3 adapter for SonicCoreNative.
 *
 * The adapter accepts PCM16 or PCM float and keeps the same format on output.
 * It is always reported active so parameter and bypass changes take effect on
 * the next buffer without requiring a player rebuild.
 */
class SonicCoreAudioProcessor : AudioProcessor {
    @Volatile var bypassed: Boolean = false
        set(value) {
            field = value
            native?.setBypassed(value)
        }

    @Volatile var eqGainsDb: FloatArray = FloatArray(10)
    @Volatile var bassDb: Float = 0f
    @Volatile var trebleDb: Float = 0f
    @Volatile var spatialMix: Float = 0f
    @Volatile var spatialRateHz: Float = 0.15f
    @Volatile var spatialWidth: Float = 0.35f
    @Volatile var panDepth: Float = 0.4f
    @Volatile var spatialRotate: Boolean = false
    @Volatile var spatialSurround: Boolean = false
    @Volatile var reverbMix: Float = 0f
    @Volatile var reverbRoom: Float = 0.5f
    @Volatile var eqEnabled: Boolean = true
    @Volatile var toneEnabled: Boolean = true
    @Volatile var spatialEnabled: Boolean = false
    @Volatile var reverbEnabled: Boolean = false
    @Volatile var limiterEnabled: Boolean = true

    private var format = AudioProcessor.AudioFormat(0, 0, C.ENCODING_INVALID)
    private var configured = false
    private var native: SonicCoreNative? = null
    private var outputBuffer: ByteBuffer = AudioProcessor.EMPTY_BUFFER
    private var ended = false
    private var parametersDirty = true

    fun markParametersChanged() {
        parametersDirty = true
    }

    override fun configure(inputFormat: AudioProcessor.AudioFormat): AudioProcessor.AudioFormat {
        if (inputFormat.encoding != C.ENCODING_PCM_16BIT &&
            inputFormat.encoding != C.ENCODING_PCM_FLOAT
        ) throw AudioProcessor.UnhandledAudioFormatException(inputFormat)

        native?.close()
        format = inputFormat
        configured = true
        native = SonicCoreNative(inputFormat.sampleRate, inputFormat.channelCount)
        native?.setBypassed(bypassed)
        parametersDirty = true
        outputBuffer = AudioProcessor.EMPTY_BUFFER
        ended = false
        return inputFormat
    }

    override fun isActive(): Boolean = configured

    override fun queueInput(inputBuffer: ByteBuffer) {
        val engine = native ?: return
        val channels = format.channelCount
        val bytesPerSample = if (format.encoding == C.ENCODING_PCM_FLOAT) 4 else 2
        val sampleCount = inputBuffer.remaining() / bytesPerSample
        val frames = sampleCount / channels
        if (frames <= 0) {
            outputBuffer = AudioProcessor.EMPTY_BUFFER
            return
        }

        if (parametersDirty) {
            engine.setParameters(
                eqGainsDb = eqGainsDb,
                bassDb = bassDb,
                trebleDb = trebleDb,
                spatialMix = spatialMix,
                spatialRateHz = spatialRateHz,
                spatialWidth = spatialWidth,
                panDepth = panDepth,
                spatialRotate = spatialRotate,
                spatialSurround = spatialSurround,
                reverbMix = reverbMix,
                reverbRoom = reverbRoom,
                eqEnabled = eqEnabled,
                toneEnabled = toneEnabled,
                spatialEnabled = spatialEnabled,
                reverbEnabled = reverbEnabled,
                limiterEnabled = limiterEnabled
            )
            parametersDirty = false
        }

        val samples = FloatArray(frames * channels)
        if (format.encoding == C.ENCODING_PCM_FLOAT) {
            for (i in samples.indices) samples[i] = inputBuffer.float
        } else {
            for (i in samples.indices) samples[i] = inputBuffer.short / 32768f
        }
        engine.process(samples, frames)

        outputBuffer = ByteBuffer.allocate(samples.size * bytesPerSample)
            .order(ByteOrder.nativeOrder())
        if (format.encoding == C.ENCODING_PCM_FLOAT) {
            for (sample in samples) outputBuffer.putFloat(sample.coerceIn(-1f, 1f))
        } else {
            for (sample in samples) {
                val value = (sample.coerceIn(-1f, 1f) * 32767f).toInt()
                outputBuffer.putShort(value.toShort())
            }
        }
        outputBuffer.flip()
    }

    override fun queueEndOfStream() {
        ended = true
    }

    override fun getOutput(): ByteBuffer {
        val result = outputBuffer
        outputBuffer = AudioProcessor.EMPTY_BUFFER
        return result
    }

    override fun isEnded(): Boolean = ended && outputBuffer === AudioProcessor.EMPTY_BUFFER

    override fun flush() {
        outputBuffer = AudioProcessor.EMPTY_BUFFER
        ended = false
        native?.reset()
    }

    override fun reset() {
        flush()
        native?.close()
        native = null
        format = AudioProcessor.AudioFormat(0, 0, C.ENCODING_INVALID)
        configured = false
        parametersDirty = true
    }

    override fun getDurationAfterProcessorApplied(durationUs: Long): Long = durationUs
}
