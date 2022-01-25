# Renderer
## Shader
- Source file directory
- Reference handle to its backend shader
	- Should be stored in parallel lists
- Expected input descriptor information
	- Uniform Buffer & its size
		- In future : include pre-defined data it can automatically watch
			- ex : the camera's view-projection matrix
	- Sampled textures
	- (etc)

## Material
- A list of its shaders
- Handles to the backend descriptor binding data
- Direct reference to any buffers' CPU-side memory
	- The memory copied to the GPU

# Renderer backend
## Shader
- A handle to the API object 

## Material
- API information required for rendering
	- ex : (vulkan) descriptor set, pipeline(s)



