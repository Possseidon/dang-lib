@echo off

for %%f in (CMakeLists.tx? *.cmake) do cmake-format -i "%%f"

call :format_directory cmake
for /D %%d in (dang-*) do call :format_directory %%d
call :format_directory doc

exit

:format_directory
<NUL set /p =Formatting %1...
for /R "%1" %%f in (CMakeLists.tx? *.cmake) do cmake-format -i "%%f"
echo  Done
goto :eof
