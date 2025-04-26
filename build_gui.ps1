# Build script for Yukon Solitaire (SDL GUI version)

# Compile the SDL GUI version
# Sørg for at output-mappen eksisterer
if (-not (Test-Path -Path "output")) {
    New-Item -ItemType Directory -Path "output" | Out-Null
}

# Kompiler GUI-versionen
gcc -o output/yukon_gui.exe yukon_logic.c yukon_gui.c -Wall -I./SDL3-devel/SDL3-3.2.10/include -L./SDL3-devel/SDL3-3.2.10/lib/x64 -lSDL3

# Kopier SDL3.dll til output-mappen
Copy-Item -Path "SDL3-devel/SDL3-3.2.10/lib/x64/SDL3.dll" -Destination "output/" -Force

# Check if the SDL GUI version compiled successfully
if ($LASTEXITCODE -eq 0) {
    Write-Host "SDL GUI version built successfully."
} else {
    Write-Host "Error building SDL GUI version. Make sure SDL3 is installed."
    exit 1
}
