# SonicCore Audio Engine

SonicCore is a portable real-time audio DSP engine and integration foundation for Android system audio effects.

It is inspired by the processing principles developed in SonicLab, but it is a separate project from the SonicLab music player. The engine is intended to become the audio foundation for:

- Android player and recorder applications;
- custom ROM audio effects services;
- AOSP/vendor Effects HAL adapters;
- rooted system-wide integrations where the platform permits it.

SonicCore does not copy or emulate Dolby Atmos, Dirac, or another proprietary product. It provides an open, measurable signal-processing core that device integrators can tune for their own hardware and output profiles.

## Design goals

- clean 32-bit float processing with explicit headroom;
- predictable bypass and dry/wet behavior;
- no allocations or locks in the real-time processing path;
- channel and sample-rate aware processing;
- deterministic offline tests and golden vectors;
- a stable C/C++ API suitable for JNI and platform adapters;
- Android-specific integrations kept outside the portable DSP core.

The engine also ships conservative profiles, deterministic regression tests,
and a benchmark smoke test. Profiles are starting points, not loudness
presets; the safety limiter remains enabled and hosts should loudness-match
comparisons.

## Initial processing chain

The first engine version defines this order:

```text
input -> input trim -> 10-band parametric EQ -> bass/treble tone
      -> spatial stage -> room reverb -> loudness/headroom stage
      -> safety limiter -> output trim -> output
```

The chain is deliberately conservative. A preset must not silently create clipping or excessive loudness, and every stage can be bypassed for A/B comparison.

## Repository layout

```text
core/       Portable C++ DSP engine and public API
tests/      Offline signal and regression tests
android/    Android adapter notes and future JNI/HAL bridges
docs/       Architecture and tuning notes
```

## Current status

This repository is the foundation phase. The public API and processing contract are being established before adding Android-specific adapters. It is not yet a drop-in system-wide effect for every Android device: system-wide routing requires integration with the target ROM's audio stack, permissions, and vendor effect configuration.

## Build the portable core

The core uses CMake and has no Android or third-party runtime dependency:

```bash
cmake -S core -B build/core
cmake --build build/core
ctest --test-dir build/core --output-on-failure
```

## Build the Android library

The Android adapter is a standalone library module. With Android SDK, NDK,
CMake 3.22.1, and Gradle available, build it with:

```bash
./gradlew :android:assembleRelease
```

The resulting AAR contains the Kotlin Media3 adapter and the native SonicCore
JNI library for the ABIs configured by the Android toolchain.

The `demo` application is a deterministic hardware test surface. It generates
a 440 Hz stereo tone and exposes bypass, spatial, room reverb, and bass controls
so DSP behavior can be checked on headphones, speakers, and Bluetooth output
before integrating a real media decoder.

System-wide ROM work is documented in [android/hal/README.md](android/hal/README.md).
It requires a target-specific AIDL/HIDL Effects HAL integration and should not
be assumed to work from an ordinary APK alone.

## Android integration direction

The Android adapter will expose the same engine through a small JNI boundary first. A separate platform adapter can later connect it to an app audio session or an AOSP/vendor Effects HAL. The adapter must handle lifecycle, session ownership, format conversion, underrun recovery, and coexistence with other vendor effects; these concerns do not belong in the DSP core.

## License

Apache-2.0. See [LICENSE](LICENSE).
