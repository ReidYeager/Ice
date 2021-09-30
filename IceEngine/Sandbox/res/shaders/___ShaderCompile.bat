@echo off

@REM for /R %%f in (*.vert) do (
@REM 	echo ==============================
@REM 	echo %%~nxf
@REM 	echo ==============================
@REM 	%VULKAN_SDK%\Bin\glslc.exe %%f -o compiled\%%~nf.vspv
@REM )

@REM for /R %%f in (*.frag) do (
@REM 	echo ==============================
@REM 	echo %%~nxf
@REM 	echo ==============================
@REM 	%VULKAN_SDK%\Bin\glslc.exe %%f -o compiled\%%~nf.fspv
@REM )

for /R %%f in (*.comp) do (
	echo ==============================
	echo %%~nxf
	echo ==============================
	%VULKAN_SDK%\Bin\glslc.exe %%f -o compiled\%%~nf.cspv
)

@REM pause

	