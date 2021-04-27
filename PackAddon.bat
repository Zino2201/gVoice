@echo off
echo Building DLLs
cmake --build build\x86 --config Release
cmake --build build\x64 --config Release
echo Preparing addon for publishing (output to build/publish)
mkdir build\publish
mkdir build\publish\gma
mkdir build\publish\dll
xcopy lua build\publish\gma\lua /E /H /I /Y
copy build\x64\bin\release\gmcl_gvoice_win64.dll build\publish\dll\gmcl_gvoice_win64.dll
copy build\x86\bin\release\gmcl_gvoice_win32.dll build\publish\dll\gmcl_gvoice_win32.dll
copy addon.json build\publish\gma\addon.json
tools\gmad.exe create -folder "%CD%\build\publish\gma" -out "%CD%\build\publish\gVoice.gma"
pause