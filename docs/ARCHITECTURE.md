# SonicCore architecture

```text
UI / ROM control surface
          |
          v
  parameter controller
          |
          v
 portable SonicCore DSP
          |
   +------+------+
   |             |
 app/JNI      ROM Effects HAL
   |             |
 playback      Android mixer
```

The portable engine owns signal processing, parameter validation, reset behavior, bypass semantics, and real-time safety. Adapters own Android lifecycle and routing. UI code must never be called from the audio callback.

## Quality rules

- process internally as float;
- reserve headroom before additive effects;
- use a final safety limiter only as a guard, not as loudness normalization;
- compare bypass and active output at matched loudness;
- test mono, stereo, 44.1 kHz, 48 kHz, and high-resolution sources;
- measure clipping, peak, RMS/LUFS, CPU time, and callback underruns.

