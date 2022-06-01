@echo off

for %%f in (CMakeLists.tx? *.cmake) do cmake-lint --suppress-decorations "%%f"

call :lint_directory cmake
for /D %%d in (dang-*) do call :lint_directory %%d
call :lint_directory doc

exit

:lint_directory
for /R "%1" %%f in (CMakeLists.tx? *.cmake) do cmake-lint --suppress-decorations "%%f"
goto :eof
