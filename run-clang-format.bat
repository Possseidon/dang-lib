@echo off

for /D %%d in (dang-*) do call :format_directory %%d

exit

:format_directory
<NUL set /p =Formatting %1...
for /R "%1" %%f in (*.cpp *.h) do clang-format -i "%%f"
echo  Done
goto :eof
