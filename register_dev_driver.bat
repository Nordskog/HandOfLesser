@echo off
setlocal

set "ROOT_DIR=%~dp0"
set "DRIVER_NAME=00handoflesser"
set "OUTPUT_DRIVER=%ROOT_DIR%output\drivers\%DRIVER_NAME%"
set "OUTPUT_APP=%OUTPUT_DRIVER%\resources\bin\win64\HandOfLesser.exe"
set "VRPATHREG="

if not exist "%OUTPUT_DRIVER%\driver.vrdrivermanifest" (
	echo ERROR: Could not find "%OUTPUT_DRIVER%\driver.vrdrivermanifest".
	echo Build the project first so the driver output exists.
	exit /b 1
)

if not exist "%OUTPUT_APP%" (
	echo ERROR: Could not find "%OUTPUT_APP%".
	echo Build the project first so the app output exists.
	exit /b 1
)

call :find_vrpathreg
if "%VRPATHREG%"=="" (
	echo ERROR: Could not find SteamVR's vrpathreg.exe.
	echo Install SteamVR or start SteamVR once before running this script.
	exit /b 1
)

echo Using vrpathreg: %VRPATHREG%
echo.

echo Removing any existing "%DRIVER_NAME%" registrations...
"%VRPATHREG%" removedriverswithname "%DRIVER_NAME%" >nul 2>nul

echo Registering project output driver...
"%VRPATHREG%" adddriver "%OUTPUT_DRIVER%"
if errorlevel 1 (
	echo ERROR: Failed to register "%OUTPUT_DRIVER%".
	exit /b 1
)

echo Ensuring activateMultipleDrivers is enabled...
"%OUTPUT_APP%" -activatemultipledrivers
if errorlevel 1 (
	echo WARNING: Failed to enable activateMultipleDrivers.
	echo The driver was still registered successfully.
	exit /b 0
)

echo.
echo Registered "%OUTPUT_DRIVER%" successfully.
exit /b 0

:find_vrpathreg
set "VRPATHREG="

for /f "tokens=2,*" %%A in ('reg query "HKLM\Software\WOW6432Node\Valve\Steam" /v InstallPath 2^>nul ^| find /i "InstallPath"') do (
	if exist "%%B\steamapps\common\SteamVR\bin\win64\vrpathreg.exe" (
		set "VRPATHREG=%%B\steamapps\common\SteamVR\bin\win64\vrpathreg.exe"
		goto :eof
	)
)

for /f "tokens=2,*" %%A in ('reg query "HKCU\Software\Valve\Steam" /v SteamPath 2^>nul ^| find /i "SteamPath"') do (
	if exist "%%B\steamapps\common\SteamVR\bin\win64\vrpathreg.exe" (
		set "VRPATHREG=%%B\steamapps\common\SteamVR\bin\win64\vrpathreg.exe"
		goto :eof
	)
)

if exist "%ProgramFiles(x86)%\Steam\steamapps\common\SteamVR\bin\win64\vrpathreg.exe" (
	set "VRPATHREG=%ProgramFiles(x86)%\Steam\steamapps\common\SteamVR\bin\win64\vrpathreg.exe"
	goto :eof
)

if exist "%ProgramFiles%\Steam\steamapps\common\SteamVR\bin\win64\vrpathreg.exe" (
	set "VRPATHREG=%ProgramFiles%\Steam\steamapps\common\SteamVR\bin\win64\vrpathreg.exe"
	goto :eof
)

goto :eof
