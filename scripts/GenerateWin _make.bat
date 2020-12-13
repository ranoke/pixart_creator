@echo off
pushd %~dp0\..\
call dependencies\bin\premake5.exe gmake
popd
PAUSE
