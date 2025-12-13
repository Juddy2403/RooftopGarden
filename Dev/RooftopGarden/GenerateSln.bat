@echo off
setlocal

REM Path to your project (relative to this script)
set PROJECT="%~dp0RooftopGarden.uproject"

echo Generating Visual Studio project files...
"C:\Program Files (x86)\Epic Games\Launcher\Engine\Binaries\Win64\UnrealVersionSelector.exe" /projectfiles %PROJECT%

echo Done!

