# Build script for Yukon Solitaire (both terminal and SDL GUI versions)

# Sørg for at output-mappen eksisterer
if (-not (Test-Path -Path "output")) {
    New-Item -ItemType Directory -Path "output" | Out-Null
}

# Compile the terminal version
Write-Host "Building terminal version..."
gcc -o output/yukon_terminal.exe yukon_logic.c yukon_terminal.c -Wall

# Check if the terminal version compiled successfully
if ($LASTEXITCODE -eq 0) {
    Write-Host "Terminal version built successfully."
} else {
    Write-Host "Error building terminal version."
    exit 1
}

# Compile the SDL GUI version (if SDL is available)
Write-Host "Building SDL GUI version..."
gcc -o output/yukon_gui.exe yukon_logic.c yukon_gui.c -Wall -I./SDL3-devel/SDL3-3.2.10/include -L./SDL3-devel/SDL3-3.2.10/lib/x64 -lSDL3

# Check if the SDL GUI version compiled successfully
if ($LASTEXITCODE -eq 0) {
    Write-Host "SDL GUI version built successfully."

    # Kopier SDL3.dll til output-mappen
    Copy-Item -Path "SDL3-devel/SDL3-3.2.10/lib/x64/SDL3.dll" -Destination "output/" -Force
} else {
    Write-Host "Warning: SDL GUI version could not be built. Make sure SDL3 is installed."
}

Write-Host "Build completed."
