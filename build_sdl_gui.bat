@echo off
REM Build script for SDL GUI application

REM Set paths to SDL
set SDL_INCLUDE=SDL3-devel\SDL3-3.2.10\include
set SDL_LIB=SDL3-devel\SDL3-3.2.10\lib\x64

REM Create output directory if it doesn't exist
if not exist output mkdir output

REM Try to compile with gcc (MinGW)
where gcc >nul 2>nul
if %ERRORLEVEL% == 0 (
    echo Using GCC compiler...
    gcc -o output\sdl_gui.exe sdl_gui.c -I%SDL_INCLUDE% -L%SDL_LIB% -lSDL3
) else (
    echo GCC not found, trying Visual Studio compiler...
    where cl >nul 2>nul
    if %ERRORLEVEL% == 0 (
        cl /nologo /W3 /EHsc /MD /I%SDL_INCLUDE% /Feoutput\sdl_gui.exe sdl_gui.c /link /LIBPATH:%SDL_LIB% SDL3.lib
    ) else (
        echo No compiler found. Please install GCC (MinGW) or Visual Studio.
        exit /b 1
    )
)

REM Copy SDL3.dll to output directory
copy %SDL_LIB%\SDL3.dll output\

echo.
if %ERRORLEVEL% == 0 (
    echo Build successful! Run 'output\sdl_gui.exe' to start the application.
) else (
    echo Build failed with error code %ERRORLEVEL%.
)
