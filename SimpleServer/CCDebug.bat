REM This script should be called from a Visual Studio Developer Command Prompt

msbuild SimpleTest\SimpleTest.vcxproj /p:Configuration="Debug (Profiling)" /t:rebuild
cd "SimpleTest\Debug (Profiling)"
vsinstr -coverage SimpleTest.exe
echo vinstr: %errorlevel%
start vsperfmon -coverage -output:SimpleTest.coverage
echo vsperfmon: %errorlevel%
SimpleTest.exe
echo SimpleTest: %errorlevel%
vsperfcmd -shutdown
echo vsperfcmd: %errorlevel%
cd ..\..