# System-wide Effects HAL integration

This folder documents the ROM/vendor integration boundary. It is not a
drop-in replacement for a device vendor HAL.

## Android 14+

Use the Stable AIDL audio effects interfaces from the target Android source
tree. The integration normally provides an effect factory and effect instance,
maps the AIDL parameter model to `soniccore::Parameters`, and feeds the native
engine from the platform effect buffer. The exact service, build target,
SEPolicy, VINTF, and audio policy changes belong to the ROM/device tree.

## Older Android releases

The target may use the HIDL effects interfaces and `audio_effects.xml`. Use the
template in this directory only as a starting point and assign a real UUID and
library name in the device tree. Do not install it alongside an existing vendor
effect without an explicit coexistence policy.

## Required validation

- verify session/device routing on speaker, wired, USB, and Bluetooth;
- compare bypass and active output at matched loudness;
- check coexistence with Dolby, Dirac, Mi Sound, and vendor volume curves;
- run VTS/CTS audio tests supplied by the target ROM;
- measure callback CPU time, underruns, peak level, and thermal behavior.

