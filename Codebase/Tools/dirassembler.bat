@echo off

REM A simple batch file that assembles images into a texture from a specified folder

set params=

if [%1] == [] (
	echo ERROR: Usage not defined. Usage: dirassembler.bat [type] [exe_path] [input_path] [output_file.dds] [format]
	exit /b
)

if [%2] == [] (
	echo ERROR: Executable not defined. Usage: dirassembler.bat [type] [exe_path] [input_path] [output_file.dds] [format]
	exit /b
)

if [%3] == [] (
	echo ERROR: Input path not defined. Usage: dirassembler.bat [type] [exe_path] [input_path] [output_file.dds] [format]
	exit /b
)

if [%4] == [] (
	echo ERROR: Output file not defined. Usage: dirassembler.bat [type] [exe_path] [input_path] [output_file.dds] [format]
	exit /b
)

if [%5] == [] (
	echo INFO: No format provided. Usage: dirassembler.bat [type] [exe_path] [input_path] [output_file.dds] [format]
) else (
	set params=-f %~5
)

echo Converting textures from "%~3" to "%~4"

set params=%~1 %params% -y -o "%~4"
setlocal enabledelayedexpansion
echo Params: !params!

for %%i in (%3\*.png) do (
	set params=!params! "%%i"
)

echo Command line: %2 !params!

%2 !params!