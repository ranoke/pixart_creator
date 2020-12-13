@echo off
pushd %~dp0\..\
call dependencies\premake5\premake5.exe vs2019
popd
PAUSE
