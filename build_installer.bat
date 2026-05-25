@echo off
setlocal

set "ROOT_DIR=%~dp0"
set "MAKENSIS=makensis.exe"
set "VERSION_FILE=%ROOT_DIR%output\version.txt"
set "DIST_DIR=%ROOT_DIR%distribution"
set "APP_EXE=%ROOT_DIR%output\drivers\00handoflesser\resources\bin\win64\HandOfLesser.exe"

if not exist "%VERSION_FILE%" (
	echo Could not find "%VERSION_FILE%".
	echo Build the project first so CMake can generate the version file.
	exit /b 1
)

if not exist "%APP_EXE%" (
	echo Could not find "%APP_EXE%".
	echo Build the project first so the app executable exists.
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

if not exist "%DIST_DIR%" mkdir "%DIST_DIR%"

pushd "%ROOT_DIR%"
"%MAKENSIS%" /DVERSION=%HOL_VERSION% installer.nsi
set "RESULT=%ERRORLEVEL%"
popd

if not "%RESULT%"=="0" exit /b %RESULT%

copy /Y "%APP_EXE%" "%DIST_DIR%\HandOfLesser.exe" >nul
if errorlevel 1 (
	echo Failed to copy HandOfLesser.exe into distribution.
	exit /b 1
)

exit /b 0
