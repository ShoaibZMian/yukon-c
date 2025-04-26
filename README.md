# Yukon-C

This repository contains a Yukon solitaire card game with a modular architecture:
1. A shared game logic component (yukon_logic.c/h)
2. A terminal-based frontend (yukon_terminal.c)
3. A GUI-based frontend using SDL3 (yukon_gui.c)

## Project Architecture

The project is structured with a clear separation between game logic and user interfaces:

- **Game Logic (Backend)**: Contains all card structures, game mechanics, and validation rules
  - `yukon_logic.c` and `yukon_logic.h`
  - No dependencies on UI components (terminal or SDL)
  - Reusable across different frontends

- **Terminal Frontend**: Simple text-based interface
  - `yukon_terminal.c`
  - Uses the game logic component

- **SDL GUI Frontend**: Graphical interface with mouse interaction
  - `yukon_gui.c`
  - Uses the game logic component
  - Requires SDL3 library

## Building the Game

You can build either the terminal version, the GUI version, or both:

### Build Both Versions
To build both the terminal and GUI versions at once:
```
.\build_all.ps1
```

### Build Terminal Version Only
To build only the terminal-based version:
```
.\build_terminal.ps1
```

### Build GUI Version Only
To build only the GUI-based version:
```
.\build_gui.ps1
```

All executables will be placed in the `output` directory.

## Running the Game

### Terminal Version
To run the terminal-based Yukon solitaire game:
```
.\output\yukon_terminal.exe
```

### GUI Version
To run the GUI-based Yukon solitaire game:
```
.\output\yukon_gui.exe
```

## Game Commands

In the terminal version, you can move cards using commands like:
- `C1:5H->C2` - Move the 5 of Hearts from column 1 to column 2
- `C3:AS->F1` - Move the Ace of Spades from column 3 to foundation pile 1

In the GUI version, you can drag and drop cards with the mouse.

## Development

If you want to modify the game:
- For game mechanics changes, edit `yukon_logic.c` and `yukon_logic.h`
- For terminal UI changes, edit `yukon_terminal.c`
- For GUI changes, edit `yukon_gui.c`

After making changes, rebuild using the appropriate build script.
