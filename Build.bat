@echo off
setlocal enabledelayedexpansion
setlocal

if defined UE_ENGINE_DIR (
    set "RUNUAT_PATH=%UE_ENGINE_DIR%\Engine\Build\BatchFiles\RunUAT.bat"
) else (
    set "RUNUAT_PATH=C:\unrealengine\UE_5.4\Engine\Build\BatchFiles\RunUAT.bat"
)

rem === Make sure path exists before running ===
if not exist "%RUNUAT_PATH%" (
rem checking the C drive for it
	set RUNUAT_PATH=C:\Program Files\Epic Games\UE_5.4\Engine\Build\BatchFiles\RunUAT.bat
)

if not exist "%RUNUAT_PATH%" (
 echo Error: RunUAT.bat not found at "%RUNUAT_PATH%"
    pause
    exit /b 1
)

if exist "%~dp0Build" (
    rmdir /s /q "%~dp0Build"
)

call "%RUNUAT_PATH%" BuildCookRun ^
-project="%~dp0Dev\RooftopGarden\RooftopGarden.uproject" ^
-noP4 ^
-platform=Win64 ^
-config=Development ^
-cook -allmaps -build -stage -pak -archive ^
-archivedirectory="%~dp0Build"

pause
endlocal
