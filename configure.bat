@rem Run CMake, pointing to sibling directories containing dependencies.

@set build_type=Release
@if not "%1"=="" set build_type=%1

@set build_bitness=64
@if not "%2"=="" set build_bitness=%2

@set FREEIMAGE_PATH=%cd%\..\..\FreeImage-vc12-x64-release-debug
@set FREEIMAGE_LIBRARY_DIR=%FREEIMAGE_PATH%\x64\%build_type%\DLL
@set FREEIMAGE_INCLUDE_DIR=%FREEIMAGE_PATH%\Source

@set GTS_PATH=%cd%\..\..\gts-0.7.0-win32
@set GTS_LIBRARY_DIR=%GTS_PATH%\gts-0.7.0
@set GTS_INCLUDE_DIR=%GTS_PATH%\gts-0.7.0\inc

@set GLIB_LIBRARY_DIR=%GTS_PATH%\glib-1.3.2
@set GLIB_INCLUDE_DIR=%GTS_PATH%\glib-1.3.2\inc

@set IGN_MATH_PATH=%cd%\..\..\ign-math\build\install\%build_type%

@set INCLUDE=%FREEIMAGE_INCLUDE_DIR%;%GTS_INCLUDE_DIR%;%GLIB_INCLUDE_DIR%;%INCLUDE%
@set LIB=%FREEIMAGE_LIBRARY_DIR%;%GTS_LIBRARY_DIR%;%GLIB_LIBRARY_DIR%;%LIB%

@echo Configuring for build type %build_type% for %build_bitness% bits
cmake -G "NMake Makefiles"^
    -DCMAKE_PREFIX_PATH="%IGN_MATH_PATH%"^
    -DFREEIMAGE_RUNS=1^
    -DCMAKE_INSTALL_PREFIX="install/%build_type%"^
    -DCMAKE_BUILD_TYPE="%build_type%"^
    -DENABLE_TESTS_COMPILATION:BOOL=True^
    ..

@if %errorlevel% neq 0 exit /b %errorlevel%
@echo Configuration complete.  To build, run `nmake`
