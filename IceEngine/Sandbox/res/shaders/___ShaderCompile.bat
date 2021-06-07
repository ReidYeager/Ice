@echo off

for /R %%f in (*.vert) do (
	echo ==============================
	echo %%~nxf
	echo ==============================
	%VULKAN_SDK%\Bin\glslc.exe %%f -o %%~nf.vspv
)

for /R %%f in (*.frag) do (
	echo ==============================
	echo %%~nxf
	echo ==============================
	%VULKAN_SDK%\Bin\glslc.exe %%f -o %%~nf.fspv
)

for /R %%f in (*.comp) do (
	echo ==============================
	echo %%~nxf
	echo ==============================
	%VULKAN_SDK%\Bin\glslc.exe %%f -o %%~nf.cspv
)

pause

	