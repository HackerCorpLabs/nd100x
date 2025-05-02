@echo off
rem Script to build the ND100X project using CMake on Windows

setlocal enabledelayedexpansion

rem Default build type
set BUILD_TYPE=Debug
set BUILD_DIR=build
set INSTALL_PREFIX=C:\Program Files\ND100X
set BUILD_WASM=0
set BUILD_DAP_TOOLS=0

rem Parse command line arguments
:parse_args
if "%~1"=="" goto :done_parsing
if /i "%~1"=="--release" (
    set BUILD_TYPE=Release
    shift
    goto :parse_args
)
if /i "%~1"=="--build-dir" (
    set BUILD_DIR=%~2
    shift
    shift
    goto :parse_args
)
if /i "%~1"=="--install-prefix" (
    set INSTALL_PREFIX=%~2
    shift
    shift
    goto :parse_args
)
if /i "%~1"=="--wasm" (
    set BUILD_WASM=1
    shift
    goto :parse_args
)
if /i "%~1"=="--with-dap-tools" (
    set BUILD_DAP_TOOLS=1
    shift
    goto :parse_args
)
if /i "%~1"=="--help" (
    echo Usage: %0 [options]
    echo Options:
    echo   --release          Build in Release mode (default: Debug)
    echo   --build-dir DIR    Set build directory (default: build)
    echo   --install-prefix P Set installation prefix (default: C:\Program Files\ND100X)
    echo   --wasm             Build WebAssembly version
    echo                      (Note: Debugger support is disabled in WASM builds)
    echo   --with-dap-tools   Build DAP test tools (dap_debugger and dap_mock_server)
    echo                      (Note: Only available on Windows/Linux builds)
    echo   --help             Show this help message
    echo.
    echo Platform-specific features:
    echo   - Windows/Linux: Automatically defines _DEBUGGER_ENABLED_ for CPU/debugger modules
    echo   - Windows/Linux: Enables DAP (Debug Adapter Protocol) integration
    echo   - WebAssembly:   No debugger support, optimized for browser performance
    exit /b 0
)
echo Unknown option: %~1
exit /b 1

:done_parsing

if %BUILD_WASM%==1 (
    rem Check if emcmake is available
    where emcmake >nul 2>&1
    if errorlevel 1 (
        echo Error: emcmake not found. Please install and activate Emscripten SDK.
        exit /b 1
    )
    
    rem Create WASM build directory
    set WASM_BUILD_DIR=%BUILD_DIR%_wasm
    if not exist "%WASM_BUILD_DIR%" mkdir "%WASM_BUILD_DIR%"
    
    echo Configuring WebAssembly build in %WASM_BUILD_DIR%...
    emcmake cmake -B "%WASM_BUILD_DIR%" -S . ^
        -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
        -DCMAKE_INSTALL_PREFIX="%INSTALL_PREFIX%" ^
        -DBUILD_DAP_TOOLS=%BUILD_DAP_TOOLS% ^
        -DBUILD_WASM=ON
    
    echo Building WebAssembly targets...
    cmake --build "%WASM_BUILD_DIR%" --config %BUILD_TYPE%
    
    rem Copy index.html template to bin directory
    echo Copying index.html template to WASM build directory...
    if not exist "%WASM_BUILD_DIR%\bin" mkdir "%WASM_BUILD_DIR%\bin"
    copy ".vscode\template\index.html" "%WASM_BUILD_DIR%\bin\"
) else (
    rem Create native build directory
    if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
    
    echo Configuring native build in %BUILD_DIR%...
    cmake -B "%BUILD_DIR%" -S . ^
        -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
        -DCMAKE_INSTALL_PREFIX="%INSTALL_PREFIX%" ^
        -DBUILD_DAP_TOOLS=%BUILD_DAP_TOOLS%
    
    echo Building native targets...
    cmake --build "%BUILD_DIR%" --config %BUILD_TYPE%
)

echo Build completed successfully!
if %BUILD_WASM%==1 (
    echo To test WASM build in a browser, run:
    echo   cd "%WASM_BUILD_DIR%\bin"
    echo   python -m http.server
    echo   # Then open http://localhost:8000/index.html in your browser
    echo To install, run: cmake --install "%WASM_BUILD_DIR%" --config %BUILD_TYPE%
) else (
    echo To install, run: cmake --install %BUILD_DIR% --config %BUILD_TYPE%
    echo Note: This build includes debugger support with _DEBUGGER_ENABLED_ flag
)

endlocal 