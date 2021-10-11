# Structure
``` c++
struct IceShader
{
	const char* name;                       // Name of source file
	IceShaderStageFlags stage;              //  The rendering stage it slots into
	double loadTime;                        // Used for hot-reloading
	IceShaderBufferInputFlags bufferInputs; // Used to load material-gloabal information into its buffer
}
```

## Components
- Name
	- The source file name without extensions
	- Extensions can be inferred by its stage flag
- Stage
	- The type of shader this is
	- Namely : Vertex, Fragment
- Load time
	- The system time this file was loaded
	- Used to compare with source file's last modified time to trigger a re-load
- Buffer inputs
	- The information that should be present in the input uniform buffer

---
# Functionality
Most functionality is rendering API specific

## Agnostic
### Initialize
1. Load source and layout files

### Test for reload
1. Read the source file's metadata
2. Compare its last modified time with `loadTime`
	1. If `loadTime` < last modified, reload

### Reload
1. Shutdown
2. Initialize

## Vulkan
![[VulkanShader#Functionality]]

## OpenGL
...

## DirectX
...