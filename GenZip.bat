@echo off
echo Building DLLs
cmake --build build\x86 --config Release
cmake --build build\x64 --config Release
echo Preparing addon for publishing (output to build/publish)
mkdir build\publish
mkdir build\publish
copy build\x64\bin\release\gmcl_gvoice_win64.dll build\publish\gmcl_gvoice_win64.dll
copy build\x86\bin\release\gmcl_gvoice_win32.dll build\publish\gmcl_gvoice_win32.dll
copy Install.bat build\publish\Install.bat
powershell Compress-Archive -Path build\publish\* -DestinationPath gVoice.zip
pause