# Ström

A vintage-styled polyphonic analog synthesizer — VST3, built with [JUCE](https://juce.com).

Ström is a classic subtractive synth (2 oscillators with unison, sub + noise, multimode
resonant filter, 2 envelopes, 2 LFOs, mod matrix, FX chain) with one unusual feature:
**The Lab**, a smart randomizer that learns your taste. Roll a random patch, judge it
(*super bad · bad · good · super good*), and Ström breeds future patches from what you
loved while steering clear of what you hated. Super-good patches are saved straight into
your **Gold vault**.

> Status: **milestone 1 of 8** — project scaffold, first sound (naive saw + ADSR),
> build pipeline. See [docs/LEARNING.md](docs/LEARNING.md) for the story so far.

## Install (Windows / FL Studio)

1. Download [`Strom-Windows.zip`](https://github.com/WFagher/strom/releases/download/latest/Strom-Windows.zip) (rolling latest build)
2. Unzip and copy the `Strom.vst3` folder into `C:\Program Files\Common Files\VST3`
3. In FL Studio: **Options → Manage plugins → Find plugins**, then add *Strom* to a channel

## Build from source (macOS/Windows/Linux)

Requires CMake ≥ 3.22 and a C++20 compiler. JUCE is fetched automatically.

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```

Artefacts land in `build/Strom_artefacts/Release/` (`VST3/` and `Standalone/`).

## License

[GPLv3](LICENSE). Built with JUCE under its open-source license.
