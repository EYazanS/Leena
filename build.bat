@echo off

if not defined DevEnvDir (
	call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
)

IF NOT EXIST build mkdir Build

pushd Build
set CommonCompilerFlags=/std:c++17 /MT /utf-8 /nologo /Gm- /GR- /EHa /Od /Oi /WX /W4 /wd4201 /wd4100 /wd4189 /wd4505 /DLeena_Internal /Z7 
set CommonLinkerFlags= -incremental:no -opt:ref  user32.lib gdi32.lib winmm.lib Xinput.lib Xinput9_1_0.lib
set CommonIncludePaths= /I C:\Users\eyaza\source\repos\Leena\Game\src

set CUR_YYYY=%date:~7,4%
set CUR_MM=%date:~3,2%
set CUR_DD=%date:~0,2%
set CUR_HH=%time:~0,2%
if %CUR_HH% lss 10 (set CUR_HH=0%time:~1,1%)

set CUR_NN=%time:~3,2%
set CUR_SS=%time:~6,2%
set CUR_MS=%time:~9,2%

set SUBFILENAME=%CUR_YYYY%%CUR_MM%%CUR_DD%%CUR_HH%%CUR_NN%%CUR_SS%

rem compilar normal x64
del *.pdb >NUL 2> NUL

cl /D_USRDLL /D_WINDLL %CommonCompilerFlags% ../Leena/src/Leena.cpp /FmLeena.map /LD /link /PDB:Leena_%SUBFILENAME%.pdb /EXPORT:GameUpdate -incremental:no

popd