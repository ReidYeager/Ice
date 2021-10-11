# Structure
![[Shaders#Structure]]

``` c++
struct IvkShader : IceShader
{
	vkShaderModule shaderModule;
}
```

## Components
- Shader module
	- Not discarded so it may be re-used for multiple [[Materials]] and for pipeline re-creation

# Functionality
## Initialize
1. Load its source & layout files
2. Create a shader module from its source
3. Read the bindings from its layout
	1. Buffer count (Should always be 1)
		1. Buffer inputs
	2. Image sampler count


## Shutdown
1. Destroy its shader module

## Re-load
1. [[#Shutdown]]
2. [[#Initialize]]

