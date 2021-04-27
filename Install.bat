@echo off

echo === gVoice Easy Installer ===
echo[
set /p gmp=GMod Path (e.g C:/Games/Steam/steamapps/common/GarrysMod): 
set /p x64=64-bit ? (y/n): 
if "%x64%" == "y" (copy gmcl_gvoice_win64.dll %gmp%\garrysmod\lua\bin\gmcl_gvoice_win64.dll) else (copy gmcl_gvoice_win32.dll %gmp%\garrysmod\lua\bin\gmcl_gvoice_win32.dll)
pause