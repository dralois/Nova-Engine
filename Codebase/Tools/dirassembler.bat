@echo off

REM A simple batch file that assembles images into a texture from a specified folder

if [%1] == [] (
	echo ERROR: Usage not defined. Usage: dirassembler.bat [type] [exe_path] [input_path] [output_file.dds]
)

if [%2] == [] (
	echo ERROR: Executable not defined. Usage: dirassembler.bat [type] [exe_path] [input_path] [output_file.dds]
)

if [%3] == [] (
	echo ERROR: Input path not defined. Usage: dirassembler.bat [type] [exe_path] [input_path] [output_file.dds]
	exit /b
)

if [%4] == [] (
	echo ERROR: Output file not defined. Usage: dirassembler.bat [type] [exe_path] [input_path] [output_file.dds]
	exit /b
)

echo %1 %2 %3 %4

echo Converting textures from "%~3" to "%~4"

set params= %~1 -y -o "%~4"
setlocal enabledelayedexpansion
echo Params: !params!

for %%i in (%3\*.png) do (
	set params=!params! "%%i"
)

echo Command line: %2 !params!

%2 !params!