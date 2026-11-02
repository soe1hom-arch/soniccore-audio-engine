# Android integration boundary

This directory is intentionally separate from the portable DSP core.

The planned adapters are:

1. JNI adapter for an Android application or service;
2. session-scoped `AudioEffect` adapter where an app owns the audio session;
3. AOSP/vendor Effects HAL adapter for a custom ROM or device build.

## JNI prototype

`SonicCoreNative.kt` is the first application-side adapter. Its native library is
built with `android/CMakeLists.txt` and exposes in-place interleaved float PCM
processing. The wrapper owns the native engine handle and must be created and
closed on the same component lifecycle that owns the audio stream.

The JNI boundary deliberately accepts already-decoded float PCM. Media decoding,
AudioTrack/Oboe callbacks, buffer sizing, and session routing remain adapter
responsibilities. Do not call UI or blocking code from the audio callback.

`SonicCoreAudioProcessor` is a Media3 `AudioProcessor` adapter for application
playback. Add it to the audio sink's processor chain after decoding and call
`markParametersChanged()` after changing live effect values. For a system-wide
effect, reuse the native engine from a platform adapter instead of trying to
route all applications through an ordinary APK.

The third option is what enables a true system-level effect. A normal APK cannot transparently process every application's output on current Android versions. The adapter must be implemented and packaged according to the target ROM's audio stack, with careful coexistence rules for existing vendor effects.
