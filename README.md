# Ice

Ice is a realtime c++ rendering engine being built because I learn by doing.  
I want to understand as many of the low level processes involved in getting an image rendered and presented to the screen as I possibly can.

The functional end goal of this project is to have a renderer that allows for realtime iteration during shader development. This includes the ability to change the rendering API (Vulkan, OpenGL, DirectX) and style of rendering (forward, deferred, pure & hybrid ray-tracing, etc.) on the fly.
> I am considering extending this desired feature-set to include offline rendering so I can experiment with higher cost rendering methodologies like path tracing, and to allow for higher resolution renders.

---

### Rendering APIs
- [x] Vulkan
- [ ] OpenGL
- [ ] DirectX

### Rendering methodologies
- [x] Forward
- [ ] Deferred
- [ ] Hybrid RT (deferred)
- [ ] Pure RT

### Notable shaders
- :(

### Notable functionality
- Hot reloading shaders

## Roadmap
- Proper input handling
	- So I can use more complicated test scenes
- UI
	- To manually edit shader parameters
	- Probably ImGui for now
- Post-processing
	- To learn how to handle multiple renderpasses

#### Shaders
- PBR
- IBL

