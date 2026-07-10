# Ström — how this synth works

A running diary of everything Ström taught us, one chapter per milestone.
Written for a producer who's new to C++ and DSP. Code excerpts refer to real
files in `src/`.

---

## Chapter 1 — It makes sound

### What a plugin actually is

A VST3 plugin is not a program. It's a bundle of code with no life of its own —
FL Studio loads it *into itself* and calls its functions. Two of those calls
matter more than all the others:

- **`processBlock()`** — "here's an empty audio buffer and the MIDI that just
  happened; fill the buffer." ([PluginProcessor.cpp](../src/PluginProcessor.cpp))
- **`createEditor()`** — "the user opened your window; hand me your UI."

The standalone Mac app we use for testing is just a thin host that does the
same thing FL does — so what works there works in FL.

### The audio callback, and why it's sacred

Sound card audio is delivered in tiny buffers — typically 128–512 samples, a
few milliseconds each — and `processBlock()` runs once per buffer, hundreds of
times a second, on a dedicated **audio thread** with a hard deadline. Miss the
deadline once and you hear a click; miss it repeatedly and audio crackles like
a broken radio.

That's why the audio thread has house rules: **no allocating memory, no locks,
no reading files, no waiting for anything.** You'll see this shape everything
we write — knob values are read via atomics, and anything slow happens on
other threads.

### Voices: how 10 fingers become sound

A synth that can play chords needs one little sound-generator per
currently-held note. Each is a **voice** ([engine/SynthVoice.cpp](../src/engine/SynthVoice.cpp)),
and Ström keeps 16. Press an 17th key and the oldest voice gets **stolen** —
you'll never notice, and it's how every hardware synth since the 80s did it.

### The oscillator: a number going around in circles

Strip away the mystique and an oscillator is this:

```cpp
phase += phaseIncrement;      // move forward a little each sample
if (phase >= 1.0)
    phase -= 1.0;             // wrapped around = one cycle completed
```

`phase` is *where we are* in one cycle of the waveform (0 → 1), and
`phaseIncrement = frequency / sampleRate` is *how fast we move*. For A4
(440 Hz) at 48 kHz, phase advances 0.00917 per sample — so one full cycle takes
about 109 samples, which is exactly 440 times per second. Pitch is just speed.

A sawtooth is then the laziest waveform imaginable: the phase itself,
stretched to -1…+1 (`2.0 * phase - 1.0`). It ramps up, snaps back, ramps up —
and that snap is what makes saws sound buzzy and bright.

### The envelope: why a note isn't a light switch

Raw oscillator on/off sounds like a doorbell. Real instruments have shape — a
piano hits hard then fades; strings swell. The classic tool is the **ADSR
envelope**: **A**ttack (fade-in time), **D**ecay (fall after the peak),
**S**ustain (the level while you hold the key), **R**elease (fade-out after
you let go). Every sample of our saw is multiplied by the envelope's current
value. Ström uses JUCE's stock ADSR for now; we'll build a nicer curved one
in a later milestone.

Small detail with a big feel: velocity is *squared* before use
(`level = velocity * velocity`), because loudness perception is roughly
logarithmic — linear velocity feels stiff.

### Parameters: one source of truth

Every knob (master volume, A, D, S, R) lives in an
`AudioProcessorValueTreeState` — JUCE's parameter registry. Registering knobs
there is what makes FL show proper names and units, record automation, and
save/restore everything with your project for free. The UI you see right now
is auto-generated from this registry; the vintage faceplate in milestone 5
will attach to exactly the same parameters.

### The flaw we shipped on purpose: aliasing

Play a high note (C6 and up) and listen carefully — there's a faint metallic
wrongness under the note, like ghost tones that don't belong to it. That's
**aliasing**. A perfect sawtooth contains infinitely many harmonics; digital
audio can only represent frequencies up to half the sample rate (the *Nyquist
limit*, ~24 kHz at 48 kHz). Our naive saw's higher harmonics don't politely
disappear — they **fold back down** into the audible range as out-of-tune
garbage.

Milestone 2 fixes this with **PolyBLEP** — a trick that rounds off the saw's
sharp snap just enough to tame the illegal harmonics without dulling the
sound. That chapter will show before/after spectrograms.
