@echo off
echo 더미 클라이언트를 시작합니다
pause
for /l %%i in (1, 1, 1) do start Debug\Client.exe 1000
