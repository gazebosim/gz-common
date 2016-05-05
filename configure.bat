@rem Run CMake, pointing to sibling directories containing dependencies.

@set build_type=Release
@if not "%1"=="" set build_type=%1

@set build_bitness=64
@if not "%2"=="" set build_bitness=%2

@set BOOST_PATH=%cd%\..\..\boost_1_56_0
@set BOOST_LIBRARY_DIR=%BOOST_PATH%\lib64-msvc-12.0

@set FREEIMAGE_PATH=%cd%\..\..\FreeImage-vc12-x64-release-debug
@set FREEIMAGE_LIBRARY_DIR=%FREEIMAGE_PATH%\x64\%build_type%\DLL
@set FREEIMAGE_INCLUDE_DIR=%FREEIMAGE_PATH%\Source

@set IGN_MATH_PATH=%cd%\..\..\ign-math\build\install\%build_type%

@set INCLUDE=%FREEIMAGE_INCLUDE_DIR%;%INCLUDE%
@set LIB=%FREEIMAGE_LIBRARY_DIR%;%BOOST_LIBRARY_DIR%;%LIB%

@echo Configuring for build type %build_type% for %build_bitness% bits
cmake -G "NMake Makefiles"^
    -DCMAKE_PREFIX_PATH="%IGN_MATH_PATH%"^
    -DFREEIMAGE_RUNS=1^
    -DBOOST_ROOT:STRING="%BOOST_PATH%"^
    -DBOOST_LIBRARYDIR:STRING="%BOOST_LIBRARY_DIR%"^
    -DCMAKE_INSTALL_PREFIX="install/%build_type%"^
    -DCMAKE_BUILD_TYPE="%build_type%"^
    ..

@if %errorlevel% neq 0 exit /b %errorlevel%
@echo Configuration complete.  To build, run `nmake`
