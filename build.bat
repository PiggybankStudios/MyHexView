@echo off
rem This batch file compiles the windows version of the platform layer and application
rem It should be run from a build directory created in the project folder

IF EXIST "build*" (
	echo Moving into build folder
	cd build
)

set TimeString=%date:~-4,4%%date:~-10,2%%date:~-7,2%%time:~0,2%%time:~3,2%%time:~6,2%
rem echo Time: %TimeString%

echo Running from %cd%
rem call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
rem call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64 -no_logo
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64

set ProjectName=MyHexView
rem set ProjectName=Playground
rem set ProjectName=GameOfLife
set LibDirectory=..\..\..\lib
set SourceDirectory=..\code
set DataDirectory=..\data

set CompilePlatform=1
set CompileApplication=1
set DebugBuild=0
set CopyToDataDirectory=1

if "%DebugBuild%"=="1" (
	set DebugDependantFlags=/MTd -DDEBUG=1
	set DebugDependantPaths=/LIBPATH:"..\lib\Win32\Debug"
	set DebugDependantLibraries=glew32d.lib
) else (
	set DebugDependantFlags=/MT -DDEBUG=0
	set DebugDependantPaths=/LIBPATH:"..\lib\Win32\Release"
	set DebugDependantLibraries=glew32.lib
)

set Definitions=-DWINDOWS_COMPILATION -DPROJECT_NAME=\"%ProjectName%\"
set IgnoredWarnings=/wd4201 /wd4100 /wd4189 /wd4996 /wd4127 /wd4505 /wd4101 /wd4702 /wd4458 /wd4324
set IncludeDirectories=/I"%LibDirectory%\mylib" /I"%LibDirectory%\glew-2.0.0\include" /I"%LibDirectory%\glfw-3.2.1\include" /I"%LibDirectory%\stb"
set CompilerFlags=/FC /Zi /EHsc /nologo /GS- /Gm- -GR- /EHa- /Fm /Od /Oi /WX /W4 %DebugDependantFlags% %Definitions% %IgnoredWarnings% %IncludeDirectories%

set LibraryDirectories=%DebugDependantPaths%
set Libraries=gdi32.lib User32.lib Shell32.lib opengl32.lib glfw3.lib Shlwapi.lib Advapi32.lib %DebugDependantLibraries%
set LinkerFlags=-incremental:no %LibraryDirectories% %Libraries%

set AppExports=/EXPORT:App_GetVersion /EXPORT:App_Initialize /EXPORT:App_Reloading /EXPORT:App_Reloaded /EXPORT:App_Update /EXPORT:App_Closing

rem Compile the resources file to generate resources.res which defines our program icon
rc /nologo %SourceDirectory%\plat_resources.rc

del *.pdb > NUL 2> NUL

if "%CompilePlatform%"=="1" (
	echo[
	
	python ..\IncrementVersionNumber.py %SourceDirectory%\win32_version.h
	
	cl /Fe%ProjectName%.exe %CompilerFlags% %IncludeDirectories% %SourceDirectory%\win32_main.cpp /link %LinkerFlags% kernel32.lib %SourceDirectory%\plat_resources.res
	
	if "%ERRORLEVEL%"=="0" (
		if "%CopyToDataDirectory%"=="1" (
			echo [Copying %ProjectName%.exe to data directory]
			XCOPY ".\%ProjectName%.exe" "%DataDirectory%\" /Y > NUL
		) else (
			echo [Platform Build Succeeded!]
		)
	) else (
		echo [Platform Build Failed! %ERRORLEVEL%]
	)
)

if "%CompileApplication%"=="1" (
	echo[
	
	python ..\IncrementVersionNumber.py %SourceDirectory%\app_version.h
	
	cl /Fe%ProjectName%.dll %CompilerFlags% %IncludeDirectories% %SourceDirectory%\app.cpp /link %LinkerFlags% %AppExports% /DLL /PDB:"%ProjectName%_%TimeString%.pdb"

	if "%ERRORLEVEL%"=="0" (
		if "%CopyToDataDirectory%"=="1" (
			echo [Copying %ProjectName%.dll to data directory]
			XCOPY ".\%ProjectName%.dll" "%DataDirectory%\" /Y > NUL
		) else (
			echo [Application Build Succeeded!]
		)
	) else (
		echo [Application Build Failed! %ERRORLEVEL%]
	)
)

rem cl /nologo %SourceDirectory%\stb_truetypeTest.cpp /FeTrueTypeTest.exe /I"%LibDirectory%\stb" 

echo[

REM Delete the object files that were created during compilation
del *.obj > NUL 2> NUL

rem echo [Done!]