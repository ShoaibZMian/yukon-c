# Yukon-C

This repository contains two card game implementations:
1. A GUI-based Yukon solitaire game using SDL3
2. A terminal-based Yukon solitaire game

## Building and Running the Card Game GUI

### Build
To build the card game GUI application, run:
```
.\build_card_game_gui.ps1
```

This PowerShell script will:
- Compile the card_game_gui.c file using either GCC or Visual Studio compiler
- Link against the SDL3 library
- Copy the SDL3.dll to the output directory

### Run
To run the card game GUI application, execute:
```
.\output\card_game_gui.exe
```

## Building and Running the Yukon Terminal

### Build
To build the terminal-based Yukon solitaire game, you can use one of the following commands:

Using GCC:
```
gcc -o output\yukon_terminal.exe yukon_terminal.c
```

Using Visual Studio:
```
cl /nologo /W3 /EHsc /MD /Feoutput\yukon_terminal.exe yukon_terminal.c
```

### Run
To run the terminal-based Yukon solitaire game, execute:
```
.\output\yukon_terminal.exe
```

## Additional Information

The project also includes an SDL GUI example (sdl_gui.c) which can be built using:
```
.\build_sdl_gui.ps1
```

And run using:
```
.\output\sdl_gui.exe
