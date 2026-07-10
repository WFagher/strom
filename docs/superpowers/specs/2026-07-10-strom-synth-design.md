# Ström — design spec (approved 2026-07-10)

## Purpose

A vintage-styled polyphonic analog (subtractive) synthesizer VST3 for FL Studio, named
**Ström**. Twin goals, equally weighted: learning how synths work (owner is new to C++
and DSP; code and docs are written tutorial-grade) and producing an instrument genuinely
usable in tracks. Signature feature: **The Lab**, a taste-learning randomizer.

Environment: development on macOS (Apple Silicon) with a Standalone build for daily
play-testing; FL Studio runs on a separate Windows PC and receives Windows VST3 builds
via GitHub Actions (rolling `latest` release). Public repo, GPLv3.

## The instrument

**Voice engine** — 16-voice polyphonic, per voice:
- OSC A + OSC B: saw / pulse (PWM) / triangle / sine; octave, semi, fine; up to 7-voice
  unison per osc with detune + stereo spread
- Sub osc (sine/square, −1/−2 oct) + noise (white/pink)
- Multimode resonant filter (LP24 / LP12 / BP / HP) with drive, key tracking, envelope amount
- ENV 1 (amp) + ENV 2 (filter/assignable), both ADSR; 2 tempo-syncable LFOs
  (sine/tri/saw/square/S&H)
- 8-slot mod matrix: sources (LFO 1/2, ENV 2, velocity, mod wheel, key track, random) →
  destinations (pitch, PWM, cutoff, resonance, osc levels, pan, …), bipolar amounts
- Glide (mono/legato/poly modes), pitch-bend range setting
- Analog character: per-voice slow drift, filter drive saturation

**FX chain** (fixed order, each bypassable, mix controls):
Drive/saturation → Chorus → tempo-synced Delay (ping-pong option) → Reverb.

**Presets**: browser panel (save/load/prev/next/banks), ~30 factory presets embedded in
the binary, user presets on disk. Full state persists inside FL projects automatically.

## The Lab (smart randomizer)

- Big RANDOM button → generates a patch → user rates: **super bad / bad / good / super good**
- Every verdict stored in a local memory file (app-data): normalized parameter snapshot +
  rating + timestamp. No cloud, no neural nets.
- Generation strategy:
  - Mostly **breed from favorites**: pick a random good/super-good patch, mutate a subset
    of parameters (magnitude set by a **Variety** knob, tame ↔ wild)
  - **Rejection re-roll** if candidate lands within distance ε of any bad/super-bad patch
    (weighted Euclidean on normalized params)
  - **Freshness re-roll** if candidate lands within ε of an already-saved favorite —
    hunts new territory
  - Fixed slice of pure (curated) randomness mixed in to avoid inbreeding
  - Cold-start baseline: curated musical ranges (never mutes volume, no 0 ms click
    envelopes, sane LFO rates)
- **Super good** additionally auto-saves the patch into the **Gold vault** folder
  (default inside Ström's user preset dir, configurable), auto-named; appears in the
  browser under a "Gold" bank
- Per-section lock toggles exclude sections from randomization
- Memory is per-machine; Gold presets are ordinary files, copyable. Sync = v2.

## UI

Custom vintage-hardware look: cream faceplate, engraved nameplate, chunky Moog-style
knobs (vector-drawn LookAndFeel, no bitmaps), retro typography, wood side cheeks,
hardware-style panel groups. The Lab gets its own panel with the four verdict
push-buttons.

## Technical approach

JUCE 8.0.14 via CMake FetchContent; C++20; formats VST3 + Standalone.
`AudioProcessorValueTreeState` with versioned ParameterIDs (~70–80 params), smoothed.
DSP: custom PolyBLEP oscillators, TPT SVF filter (cascaded for LP24) with drive, custom
exponential ADSR, juce::dsp chorus/delay/reverb for v1, block-rate modulation,
per-voice drift. CI: GitHub Actions (windows + macos), pluginval at high strictness,
artifacts + rolling `latest` prerelease with a stable download URL.

Source layout: `src/engine/` (voices, osc, filter, env, LFO, mod matrix), `src/fx/`,
`src/lab/` (Randomizer, TasteMemory, GoldVault), `src/presets/`, `src/ui/`.

## Milestones

1. **It makes sound** — scaffold; 1 saw osc, poly voices, amp ADSR, volume; standalone
   runs on Mac; CI green; *loads and plays in FL on the PC* (pipeline proven early)
2. **It sounds analog** — full OSC A/B, unison, sub + noise, filter + ENV 2, glide, drift
3. **It moves** — LFOs + mod matrix
4. **It's produced** — FX chain
5. **It looks vintage** — full custom UI
6. **It's an instrument** — preset infrastructure + factory bank
7. **The Lab** — randomizer, ratings, taste memory, Gold vault, Variety, locks
8. **It ships** — validation, CPU pass, docs, v1.0 tag

## Verification

Per milestone: clean build; standalone played by ear (owner) + offline MIDI→WAV renders
analyzed numerically (tuning, aliasing floor, clicks/NaNs). CI must be green (pluginval).
FL Studio on the PC checked at milestone 1 and every even milestone (scan, sound,
automation, state recall). Unit tests where math is testable: envelope timing, tuning,
mod-matrix routing, Lab distance/re-roll logic.
