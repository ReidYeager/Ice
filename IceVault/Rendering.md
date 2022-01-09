
## Renderpasses
Hopefully this covers all the use-cases I'll encounter while writing shaders

- Pre-pass
	- Things that are used in subsequent renderpasses
		- Shadow depth maps, compute shaders?, etc.
- Geometry pass
	- A deferred geometry pass
- Post-processing pass
	- Apply effects onto the swapchain image created in the previous pass
- UI pass
	- Basically just for ImGui


