@echo off
setlocal

set "ROOT_DIR=%~dp0"
set "MAKENSIS=makensis.exe"
set "VERSION_FILE=%ROOT_DIR%output\version.txt"

if not exist "%VERSION_FILE%" (
	echo Could not find "%VERSION_FILE%".
	echo Run build.bat before building the installer so CMake can generate the version file.
	exit /b 1
)

set /p HOL_VERSION=<"%VERSION_FILE%"

where makensis.exe >nul 2>nul
if errorlevel 1 (
	if exist "%ProgramFiles(x86)%\NSIS\makensis.exe" (
		set "MAKENSIS=%ProgramFiles(x86)%\NSIS\makensis.exe"
	) else if exist "%ProgramFiles%\NSIS\makensis.exe" (
		set "MAKENSIS=%ProgramFiles%\NSIS\makensis.exe"
	) else (
		echo Could not find makensis.exe. Install NSIS or add it to PATH.
		exit /b 1
	)
)

pushd "%ROOT_DIR%install"
"%MAKENSIS%" /DVERSION=%HOL_VERSION% installer.nsi
set "RESULT=%ERRORLEVEL%"
popd

exit /b %RESULT%
