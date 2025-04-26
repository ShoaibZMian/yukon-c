# Build script for Yukon Solitaire (terminal version)

# Compile the terminal version
# Sørg for at output-mappen eksisterer
if (-not (Test-Path -Path "output")) {
    New-Item -ItemType Directory -Path "output" | Out-Null
}

# Kompiler terminal-versionen
gcc -o output/yukon_terminal.exe yukon_logic.c yukon_terminal.c -Wall

# Check if the terminal version compiled successfully
if ($LASTEXITCODE -eq 0) {
    Write-Host "Terminal version built successfully."
} else {
    Write-Host "Error building terminal version."
    exit 1
}
