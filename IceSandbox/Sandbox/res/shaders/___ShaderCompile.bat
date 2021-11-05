@echo off

for /R %%f in (*.vert) do (
	echo ==============================
	echo %%~nxf
	echo ==============================
	%VULKAN_SDK%\Bin\glslc.exe %%f -o compiled\%%~nf.vert.spv
)

for /R %%f in (*.frag) do (
 	echo ==============================
 	echo %%~nxf
	echo ==============================
	%VULKAN_SDK%\Bin\glslc.exe %%f -o compiled\%%~nf.frag.spv
)

for /R %%f in (*.comp) do (
	echo ==============================
	echo %%~nxf
	echo ==============================
	%VULKAN_SDK%\Bin\glslc.exe %%f -o compiled\%%~nf.comp.spv
)

pause

	