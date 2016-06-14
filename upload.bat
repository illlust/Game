setlocal

set zip=
for /f "tokens=1 delims=." %%i in ('dir /b *.sln') do set zip=%%i.zip
del /q %zip%
alzip -a -an"ftp.txt,*.exe,*.pdb,*.map,*.ilk,*.res,*.dep,*.htm,*.manifest,*.obj,*.pch,*.tlh,*.tli,*.idb,*.ncb,*.png,*.bak,*.log,*.dll,*.dmp,*.zip" *.* %zip%
ftp -s:ftp.txt

pause
