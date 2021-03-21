@echo off

if not defined DevEnvDir (
	call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
)

IF NOT EXIST build mkdir Build

pushd Build
set CommonCompilerFlags =/std:c++17 /utf-8 -MTd /nologo /Gm- /GR- /EHa /Od /Oi /WX /W4 /wd4201 /wd4100 /wd4189 /wd4505 /DLeena_Internal /Z7
set CommonIncludePaths = /I ..\Leena\src
set CommonLinkerFlags = -incremental:no -opt:ref winmm.lib user32.lib gdi32.lib winmm.lib Xinput.lib Xinput9_1_0.lib

set CUR_HH=%time:~0,2%
if %CUR_HH% lss 10 (set CUR_HH=0%time:~1,1%)

set CUR_NN=%time:~3,2%
set CUR_SS=%time:~6,2%
set SUBFILENAME=%date%%CUR_HH%%CUR_NN%%CUR_SS%

rem compilar normal x64
del *.pdb >NUL 2> NUL

cl %CommonCompilerFlags% %CommonIncludePaths% ..\Leena\src\Leena.cpp -FmLeena.map -LD /link %CommonLinkerFlags% -PDB:Leena_%SUBFILENAME%.pdb -EXPORT:GameUpdate

popd