# MIDI to G-code Converter

This application converts MIDI files into G-code that can be run on 3D printers to play music using the stepper motors.

## Building the Project

1. Make sure you have CMake installed (version 3.10 or higher)
2. Create a build directory and navigate to it:
```bash
mkdir build
cd build
```
3. Generate the build files:
```bash
cmake ..
```
4. Build the project:
```bash
cmake --build .
```

## Usage

```bash
./midi2gcode <input_midi_file> <output_gcode_file>
```

Example:
```bash
./midi2gcode song.mid song.gcode
```

## How it Works

The program works by:
1. Reading and parsing the MIDI file to extract note information
2. Converting MIDI notes to frequencies
3. Mapping frequencies to stepper motor movements
4. Generating G-code commands that will move the printer's X-axis at speeds corresponding to the musical notes

## Configuration

You can modify the following parameters in the code:
- Maximum speed (default: 200mm/s)
- Steps per millimeter (default: 80 steps/mm)

## Safety Notes

- Before running the generated G-code, make sure:
  - Your printer's firmware has thermal runaway protection enabled
  - The hotend and bed temperatures are set to 0
  - The movement speeds are within your printer's capabilities
  - The movement range is within your printer's bounds

## Limitations

- Currently only uses the X-axis for note playing
- Simplified frequency-to-speed conversion
- May need adjustment based on specific printer characteristics
