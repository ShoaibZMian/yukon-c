# PowerShell script to build SDL GUI application

# Set paths to SDL
$SDL_INCLUDE = "SDL3-devel\SDL3-3.2.10\include"
$SDL_LIB = "SDL3-devel\SDL3-3.2.10\lib\x64"

# Create output directory if it doesn't exist
if (-not (Test-Path "output")) {
    New-Item -ItemType Directory -Path "output" | Out-Null
}

# Try to compile with gcc (MinGW)
$gcc_exists = $null -ne (Get-Command "gcc" -ErrorAction SilentlyContinue)
if ($gcc_exists) {
    Write-Host "Using GCC compiler..."
    gcc -o output\sdl_gui.exe sdl_gui.c -I"$SDL_INCLUDE" -L"$SDL_LIB" -lSDL3
}
else {
    Write-Host "GCC not found, trying Visual Studio compiler..."
    $cl_exists = $null -ne (Get-Command "cl" -ErrorAction SilentlyContinue)
    if ($cl_exists) {
        cl /nologo /W3 /EHsc /MD /I"$SDL_INCLUDE" /Feoutput\sdl_gui.exe sdl_gui.c /link /LIBPATH:"$SDL_LIB" SDL3.lib
    }
    else {
        Write-Host "No compiler found. Please install GCC (MinGW) or Visual Studio."
        exit 1
    }
}

# Copy SDL3.dll to output directory
Copy-Item "$SDL_LIB\SDL3.dll" -Destination "output\"

Write-Host ""
if ($LASTEXITCODE -eq 0) {
    Write-Host "Build successful! Run 'output\sdl_gui.exe' to start the application."
}
else {
    Write-Host "Build failed with error code $LASTEXITCODE."
}
