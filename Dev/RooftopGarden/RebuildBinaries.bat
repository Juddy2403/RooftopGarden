@echo off
setlocal

REM Path to your project (relative to this script)
set PROJECT="%~dp0RooftopGarden.uproject"
set PROJECT_DIR=%~dp0

REM === Delete folders if they exist ===
if exist "%PROJECT_DIR%Intermediate" (
    rmdir /s /q "%PROJECT_DIR%Intermediate"
)

if exist "%PROJECT_DIR%Binaries" (
    rmdir /s /q "%PROJECT_DIR%Binaries"
)

if exist "%PROJECT_DIR%.idea" (
    rmdir /s /q "%PROJECT_DIR%.idea"
)

if exist "%PROJECT_DIR%.vs" (
    rmdir /s /q "%PROJECT_DIR%.vs"
)

if exist "%PROJECT_DIR%DerivedDataCache" (
    rmdir /s /q "%PROJECT_DIR%DerivedDataCache"
)

echo Generating Visual Studio project files...
"C:\Program Files (x86)\Epic Games\Launcher\Engine\Binaries\Win64\UnrealVersionSelector.exe" /projectfiles %PROJECT%

echo Done!

