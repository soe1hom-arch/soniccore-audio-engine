# SonicCore Root Module

This is the first Magisk/KernelSU integration layer for SonicCore. It is
disabled by default and intentionally does not overwrite a device's audio
effects configuration during installation.

The native SonicCore engine is portable, but a system-wide effect still needs
a device adapter. Depending on the ROM, that adapter may target a legacy
effect library, HIDL, or Android 14+ AIDL Effects HAL. The module cannot safely
guess the correct vendor library path or effect UUID.

This package provides the reversible module lifecycle and a place for the
target-specific adapter. It is not yet a universal system-wide effect by
itself.

## Controls

```text
persist.soniccore.enabled=0
persist.soniccore.adapter=none
```

Enable only after a compatible adapter is present:

```bash
su -c 'resetprop -n persist.soniccore.enabled 1'
su -c 'resetprop -n persist.soniccore.adapter <adapter-name>'
```

Disable and reboot to return to the stock path:

```bash
su -c 'resetprop -n persist.soniccore.enabled 0'
```

The stock audio configuration is backed up under
`/data/adb/soniccore/backup/`. Installation does not modify vendor files.

