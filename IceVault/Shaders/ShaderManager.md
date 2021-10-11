# Structure
``` c++
class IceShaderManager
{
private:
	IceShaderManager* instance;
	vector<IceShader*> shaders;
}
```

---
# Functionality
## Load Shader
Changes depending on the active render API
### Vulkan
1. [[VulkanShader#Initialize]]

## Shutdown
1. Loop through all shaders
	1. Run the shader's shutdown
2. Clear the loaded shaders
3. {Free any allocated memory}

## Restart
Will be used when render API swapping is in-place
1. Loop through all shaders
	1. Run the shader's shutdown
	2. Save a copy of the base [[Shaders]] information
	3. Destroy the shader
	4. Create a new shader with the new render API
	5. Initialize the new shader with the copied base information

## Test for reloads
1. Loop through all shaders
	1. Call the shader's [[Shaders#Test for reload|Test for reload]] function

## Get Shader
### (int index)
1. Return a handle to the shader in the input index

### (string name, IceShaderStageFlags stage)
1. Loop through all shaders
	1. `if (name == shader.name && stage & shader.stage)`
	2. Return a handle to this shader
	- Note : This returns the first shader with the name in one of the input stages -- NOT all applicable shaders

