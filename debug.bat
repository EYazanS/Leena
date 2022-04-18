@echo off

if not defined DevEnvDir (
	call "compiler.bat"
)

devenv .\build\main.exe