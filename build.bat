@echo off
setlocal

echo.
echo ================================
echo   Modularity - VS 2026 Build
echo ================================
echo.

git submodule update --init --recursive

:: Clean old build
if exist build rmdir /s /q build

echo [INFO] Creating fresh build directory...
mkdir build
cd build

echo [INFO] Configuring with CMake (Visual Studio 18 2026)...
cmake -G "Visual Studio 18 2026" -A x64 ..

if errorlevel 1 (
    echo.
    echo [ERROR] CMake configuration failed!
    pause
    exit /b 1
)

echo [INFO] Building Release (using all CPU cores)...
cmake --build . --config Release -- /m

if errorlevel 1 (
    echo.
    echo [ERROR] Build failed!
    pause
    exit /b 1
)

echo [INFO] Copying Resources...
xcopy /e /i /y "..\Resources" "Resources\" >nul

echo.
echo =========================================
echo   SUCCESS! Your game is ready!
echo   At build\Release\main.exe
echo =========================================
echo.
pause