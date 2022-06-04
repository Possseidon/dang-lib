@echo off

set ANY_ERRORS=0

for %%f in (CMakeLists.tx? *.cmake) do cmake-lint --suppress-decorations "%%f" || set ANY_ERRORS=1

call :lint_directory cmake
for /D %%d in (dang-*) do call :lint_directory %%d
call :lint_directory doc

exit /b %ANY_ERRORS%

:lint_directory
for /R "%1" %%f in (CMakeLists.tx? *.cmake) do cmake-lint --suppress-decorations "%%f" || set ANY_ERRORS=1
goto :eof
