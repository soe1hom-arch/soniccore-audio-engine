/*
 * Copyright 2026 SonicCore contributors
 * SPDX-License-Identifier: Apache-2.0
 */

package com.soniccore.android

/** Thin JNI boundary for the portable SonicCore real-time engine. */
class SonicCoreNative(
    sampleRate: Int = 48_000,
    private val channels: Int = 2
) : AutoCloseable {
    private var handle = 0L

    init {
        require(channels > 0) { "SonicCore requires at least one channel" }
        handle = nativeCreate(sampleRate, channels)
        require(handle != 0L) { "SonicCore native engine could not be created" }
    }

    fun setBypassed(bypassed: Boolean) {
        checkOpen()
        nativeSetBypassed(handle, bypassed)
    }

    fun setParameters(
        eqGainsDb: FloatArray = FloatArray(10),
        bassDb: Float = 0f,
        trebleDb: Float = 0f,
        spatialMix: Float = 0f,
        spatialRateHz: Float = 0.15f,
        spatialWidth: Float = 0.35f,
        panDepth: Float = 0.4f,
        spatialRotate: Boolean = false,
        spatialSurround: Boolean = false,
        reverbMix: Float = 0f,
        reverbRoom: Float = 0.5f,
        eqEnabled: Boolean = true,
        toneEnabled: Boolean = true,
        spatialEnabled: Boolean = false,
        reverbEnabled: Boolean = false,
        limiterEnabled: Boolean = true
    ) {
        checkOpen()
        require(eqGainsDb.size >= 10) { "SonicCore requires ten EQ band values" }
        nativeSetParameters(
            handle, eqGainsDb, bassDb, trebleDb, spatialMix, spatialRateHz,
            spatialWidth, panDepth, spatialRotate, spatialSurround,
            reverbMix, reverbRoom, eqEnabled, toneEnabled, spatialEnabled,
            reverbEnabled, limiterEnabled
        )
    }

    /** Processes interleaved float PCM in-place. */
    fun process(interleaved: FloatArray, frames: Int = interleaved.size / channels) {
        checkOpen()
        require(frames >= 0 && frames * channels <= interleaved.size)
        nativeProcess(handle, interleaved, frames)
    }

    fun reset() {
        checkOpen()
        nativeReset(handle)
    }

    override fun close() {
        if (handle != 0L) {
            nativeDestroy(handle)
            handle = 0L
        }
    }

    private fun checkOpen() {
        check(handle != 0L) { "SonicCoreNative is already closed" }
    }

    private companion object {
        init { System.loadLibrary("soniccore_jni") }

        @JvmStatic external fun nativeCreate(sampleRate: Int, channels: Int): Long
        @JvmStatic external fun nativeDestroy(handle: Long)
        @JvmStatic external fun nativeSetBypassed(handle: Long, bypassed: Boolean)
        @JvmStatic external fun nativeReset(handle: Long)
        @JvmStatic external fun nativeSetParameters(
            handle: Long,
            eqGainsDb: FloatArray,
            bassDb: Float,
            trebleDb: Float,
            spatialMix: Float,
            spatialRateHz: Float,
            spatialWidth: Float,
            panDepth: Float,
            spatialRotate: Boolean,
            spatialSurround: Boolean,
            reverbMix: Float,
            reverbRoom: Float,
            eqEnabled: Boolean,
            toneEnabled: Boolean,
            spatialEnabled: Boolean,
            reverbEnabled: Boolean,
            limiterEnabled: Boolean
        )
        @JvmStatic external fun nativeProcess(handle: Long, samples: FloatArray, frames: Int)
    }
}
