# MIDI to G-code Converter

A sophisticated application that converts MIDI files into G-code patterns for 3D printers, featuring a modern graphical user interface and real-time visualization.

## Features

- **Graphical User Interface**: Built with Dear ImGui for a modern, user-friendly experience
- **Real-time MIDI Playback**: Preview your MIDI files before conversion
- **G-code Visualization**: See the generated pattern before sending it to your printer
- **Advanced Pattern Generation**: Creates spiral patterns where:
  - Note timing maps to angle
  - Note velocity affects radius (louder = larger circles)
  - Note pitch controls Z height
  - Note frequency determines movement speed
- **Printer Settings Management**: Save and load printer profiles
- **Dark/Light Theme Support**: Customizable UI appearance

## Prerequisites

- CMake (version 3.10 or higher)
- C++ compiler with C++17 support
- Visual Studio 2019 or later (for Windows)
- OpenGL 3.3 capable graphics card and drivers

## Dependencies (automatically handled by CMake)

- Dear ImGui: UI framework
- GLFW: Window management and OpenGL context
- GLAD: OpenGL function loading
- PortMidi: MIDI file parsing and playback
- GLM: 3D mathematics
- nlohmann/json: Settings management

## Building the Project

1. Clone the repository:
```bash
git clone https://github.com/yourusername/midi-2-gcode.git
cd midi-2-gcode
```

2. Create and navigate to the build directory:
```bash
mkdir build
cd build
```

3. Generate the build files:
```bash
cmake ..
```

4. Build the project:
- On Windows with Visual Studio:
```bash
cmake --build . --config Release
```
- On Linux/macOS:
```bash
cmake --build .
```

## Usage

1. Launch the application:
```bash
./midi2gcode
```

2. Using the GUI:
   - Click "Open MIDI File" to load your MIDI file
   - Use the playback controls to preview the music
   - Adjust printer settings if needed
   - Click "Generate G-code" to create the pattern
   - Preview the pattern in the visualization window
   - Save the G-code to your desired location

## Configuration

### Printer Settings
- Bed Size (X/Y dimensions)
- Maximum Speed (mm/s)
- Steps per Millimeter
- Acceleration (mm/s²)
- Jerk Settings

### Pattern Settings
- Base Pattern Size
- Z-height Range
- Movement Speed Range

### Application Settings
- UI Theme (Dark/Light)
- Output Directory
- Default Printer Profile

## File Locations

- Settings are saved in the user's AppData directory
- Generated G-code files are saved in Documents/MIDI2GCode by default
- Printer profiles are stored with the application settings

## Safety Notes

- Before running the generated G-code:
  - Ensure your printer's firmware has thermal runaway protection enabled
  - Verify the hotend and bed temperatures are set to 0
  - Check that movement speeds and accelerations are within your printer's capabilities
  - Confirm the pattern fits within your printer's build volume
  - Test with slower speeds first

## Technical Details

- MIDI parsing handles:
  - Multiple tracks
  - Tempo changes
  - Note velocity and duration
  - Proper timing calculations
- G-code generation includes:
  - Proper acceleration control
  - Synchronized multi-axis movement
  - Optimized path planning
  - Detailed comments for debugging

## Contributing

Contributions are welcome! Please feel free to submit pull requests or create issues for bugs and feature requests.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Dear ImGui for the excellent UI framework
- The 3D printing community for inspiration and testing
