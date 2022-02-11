@echo off

if not defined DevEnvDir (
	call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
)

set CommonCompilerFlags=/std:c++17 -MTd -nologo -fp:fast -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -DLeena_Internal=1 -DLeena_Slow=1 -DLeena_Win32=1 -FC -Z7
set CommonLinkerFlags=-incremental:no -opt:ref Ole32.lib user32.lib Gdi32.lib winmm.lib Xinput.lib Xinput9_1_0.lib

IF NOT EXIST build mkdir build
pushd build

REM 64-bit build
del *.pdb > NUL 2> NUL

REM Optimization switches /O2
echo WAITING FOR PDB > lock.tmp
cl %CommonCompilerFlags% ..\src\Leena.cpp -FmLeena.map -LD /link -incremental:no -opt:ref -PDB:Leena_%random%.pdb -EXPORT:GameUpdateAudio -EXPORT:GameUpdateAndRender
del lock.tmp
cl %CommonCompilerFlags% -DLeena_Internal ..\src\main.cpp -Fmwin32_Leena.map /link %CommonLinkerFlags%
popd