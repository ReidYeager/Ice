
# Structure
``` c++
struct Material
{
	vector<image*> textures;
	Shader* shaders[3]; // Vertex, Fragment, Compute
	IceShaderStageFlags shaderStages; // The stages this material acutally uses
	IceShaderBufferInputFlags bufferInputs; // The cumulative inputs of its shaders
	Buffer buffer; // The buffer passed to its shaders
}
```

## Components
- Textures
	- Handles to [[Textures]] for sampling
- Shaders 
	- Where the Vertex, Fragment, and Compute shader handles will be stored
	- [ ] Re-think this to more easily implement multiple subpasses, etc #todo #someday
- ShaderStages
	- The stages this material acutally uses
- BufferInputs
	- The cumulative inputs of its shaders
	- The information that should be present in its input uniform buffer
	- Acts as a representation of the buffer's size
		- Number of active flags * 16 bytes
	- Separate shader uniforms via sets?
		- Set 0 : vertex
		- Set 1 : fragment
- Buffer
	- The buffer passed to its shaders
	- Shall contain the information requested by its BufferInputs

---
# Functionality
All functionality is render API specific

## Vulkan
![[VulkanMaterial]]

