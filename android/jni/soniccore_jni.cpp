// Copyright 2026 SonicCore contributors
// SPDX-License-Identifier: Apache-2.0

#include <jni.h>

#include <algorithm>
#include <cstdint>
#include <memory>

#include "soniccore/sonic_core.h"

namespace {

soniccore::Engine* from_handle(jlong handle) {
    return reinterpret_cast<soniccore::Engine*>(static_cast<intptr_t>(handle));
}

}  // namespace

extern "C" {

JNIEXPORT jlong JNICALL
Java_com_soniccore_android_SonicCoreNative_nativeCreate(
    JNIEnv*, jclass, jint sample_rate, jint channels) {
    soniccore::Config config;
    config.sample_rate = std::max(1, static_cast<int>(sample_rate));
    config.channels = std::max(1, static_cast<int>(channels));
    return static_cast<jlong>(reinterpret_cast<intptr_t>(new soniccore::Engine(config)));
}

JNIEXPORT void JNICALL
Java_com_soniccore_android_SonicCoreNative_nativeDestroy(
    JNIEnv*, jclass, jlong handle) {
    delete from_handle(handle);
}

JNIEXPORT void JNICALL
Java_com_soniccore_android_SonicCoreNative_nativeSetBypassed(
    JNIEnv*, jclass, jlong handle, jboolean bypassed) {
    if (auto* engine = from_handle(handle)) engine->set_bypassed(bypassed == JNI_TRUE);
}

JNIEXPORT void JNICALL
Java_com_soniccore_android_SonicCoreNative_nativeReset(
    JNIEnv*, jclass, jlong handle) {
    if (auto* engine = from_handle(handle)) engine->reset();
}

JNIEXPORT void JNICALL
Java_com_soniccore_android_SonicCoreNative_nativeSetParameters(
    JNIEnv* env, jclass, jlong handle, jfloatArray eq_gains,
    jfloat bass_db, jfloat treble_db, jfloat spatial_mix,
    jfloat spatial_rate_hz, jfloat spatial_width, jfloat pan_depth,
    jboolean spatial_rotate, jboolean spatial_surround,
    jfloat reverb_mix, jfloat reverb_room, jboolean eq_enabled,
    jboolean tone_enabled, jboolean spatial_enabled, jboolean reverb_enabled,
    jboolean limiter_enabled) {
    auto* engine = from_handle(handle);
    if (engine == nullptr) return;

    soniccore::Parameters parameters;
    if (eq_gains != nullptr) {
        const jsize count = env->GetArrayLength(eq_gains);
        const jsize copy_count = std::min<jsize>(count, 10);
        jfloat values[10]{};
        env->GetFloatArrayRegion(eq_gains, 0, copy_count, values);
        for (jsize i = 0; i < copy_count; ++i) parameters.eq_gain_db[i] = values[i];
    }
    parameters.bass_db = bass_db;
    parameters.treble_db = treble_db;
    parameters.spatial_mix = spatial_mix;
    parameters.spatial_rate_hz = spatial_rate_hz;
    parameters.spatial_width = spatial_width;
    parameters.spatial_pan_depth = pan_depth;
    parameters.spatial_rotate = spatial_rotate == JNI_TRUE;
    parameters.spatial_surround = spatial_surround == JNI_TRUE;
    parameters.reverb_mix = reverb_mix;
    parameters.reverb_room = reverb_room;
    engine->set_parameters(parameters);
    engine->set_effects_enabled(
        eq_enabled == JNI_TRUE,
        tone_enabled == JNI_TRUE,
        spatial_enabled == JNI_TRUE,
        reverb_enabled == JNI_TRUE,
        limiter_enabled == JNI_TRUE);
}

JNIEXPORT void JNICALL
Java_com_soniccore_android_SonicCoreNative_nativeProcess(
    JNIEnv* env, jclass, jlong handle, jfloatArray samples, jint frames) {
    auto* engine = from_handle(handle);
    if (engine == nullptr || samples == nullptr || frames <= 0) return;

    jfloat* data = static_cast<jfloat*>(env->GetPrimitiveArrayCritical(samples, nullptr));
    if (data == nullptr) return;
    engine->process(data, static_cast<std::size_t>(frames));
    env->ReleasePrimitiveArrayCritical(samples, data, 0);
}

}  // extern "C"
