# R2R MUSIC VOX 2 MIDI

A JUCE-based Windows x64 VST3/Standalone starter project by DJ R2R.

## Features in this prototype
- Real-time monophonic pitch-to-MIDI note generation from the plugin audio input.
- Audio-file analysis and Standard MIDI File export.
- WAV, AIFF, FLAC and MP3 input through JUCE's basic audio formats (actual codec support depends on the JUCE/platform build).
- Adjustable pitch-confidence threshold.
- Custom dark interface with violet, cyan and lime accents.

## Build on Windows
1. Install Visual Studio 2022 with **Desktop development with C++**.
2. Install CMake 3.22+ and Git.
3. Configure and build:
   `cmake -S . -B build -A x64`
   `cmake --build build --config Release --target R2RMusicVox2Midi_VST3`
4. Find the VST3 bundle under `build/R2RMusicVox2Midi_artefacts/Release/VST3/`.

## GitHub Actions
Push to `main` or run the **Windows VST3 Build** workflow manually. A successful run uploads the VST3 bundle as a workflow artifact.

## Important limitations
This is an early monophonic pitch-tracking prototype, not a polyphonic vocal transcription engine. Real-time MIDI is generated from audio entering the plugin; configure the DAW's audio/MIDI routing accordingly. The offline converter uses a simple autocorrelation detector and needs refinement for vibrato, breathy vocals, noisy mixes, chords and complex material. Test the plugin in a DAW before production use.

## License
JUCE is fetched separately under its own license. Review JUCE licensing terms before distributing binaries commercially.


## Build status
The MIDI writer uses JUCE's `MidiFile::writeTo` API.


## DJ R2R Optical Tube Compressor (new VST3 target)
The repository now also contains a separate **DJ R2R Optical Tube Compressor** VST3/Standalone target. It does not replace the VOX 2 MIDI target.

### Controls
- Peak Reduction: compression threshold/amount on a 0–50 scale.
- Output Gain: output trim, approximately -6 to +6 dB.
- Tube Warmth: adjustable soft saturation.
- COMPRESS / LIMIT: selects 4:1 compression or high-ratio limiting.
- Power: bypass.
- Vintage-style VU display: gain-reduction indication.

### Build
`cmake -S . -B build -A x64`
`cmake --build build --config Release --target R2ROpticalTubeCompressor_VST3`

The interface is an original R2R-branded interpretation of the supplied front-panel concept. The DSP is a practical starter compressor/limiter with envelope-based gain reduction and tube-style saturation, not a component-level analog circuit emulation. Validate levels and behavior in your DAW before release.
