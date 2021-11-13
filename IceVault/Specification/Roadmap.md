# 1. Rendering one static quad
Hard-code the minimum required code to get a single quad rendering on-screen
Although everything will be hard-coded, separate everything into its own function call.
- Will allow future abstraction changes to be made easily

- Use `MARK_MODEL_DEPENDENT` to note sections that may need to change based on the model
- Use `MARK_RENDERING_SETTINTG` to note sections that I may want to be able to change dynamically later

No controllable camera, no model loading, no shader loading, no materials, etc.

1. Create window
	- GLFW
2. Initialize Vulkan
3. Enter render loop
- Don't bother adding an exit condition for the render loop for now.
	- Will be added later when input is added

# 2. Model
