# Root module rollout plan

The root module is a delivery and safety layer, not a replacement for the
device's Audio HAL. It has three adapter stages:

1. **Detection** — identify Android version, vendor, audio config locations,
   existing effect libraries, and whether the device uses HIDL or AIDL.
2. **Device adapter** — implement the matching effect entry point and map its
   parameters to SonicCore. This adapter is the only component allowed to
   modify the target audio path.
3. **Validation** — verify boot, calls, notifications, media, Bluetooth,
   wired output, volume curves, thermal behavior, and clean disable/restore.

The generic module stays disabled until stage 2 exists. This prevents a
universal ZIP from silently breaking audio on an unknown ROM.

