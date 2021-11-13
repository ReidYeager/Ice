I want this shit to be easy to use
- That means ~~abstraction~~ **separation**
	- Separate everything, use handles for interactions

## Things I want to have game control of
Fill buffer information
~~Recreate shaders when their source code changes~~

#### Rendering management
- Change how rendering works
	- Add renderpasses
	- Add subpasses
	- Change rasterization / multisampling / color adding / etc. information in pipeline

#### Object management
- Set transform
	- Set buffer information
- Set material
	- Changes its position in the rendering loop
- Set textures
	- Set image descriptor information
~~- Set Model
	- Set buffer information
	- Set pipeline information~~
- Push information to its material
	- Set buffer information
	- (Object/Material specific buffer)

#### Camera
- Set transform
	- Set buffer information
- Set projection settings
	- Set buffer information

> Don't bother adding actual input handling or anything like that.
> That can come ***WAY*** after getting the rest of it working properly
> In the meantime, set the camera's transform & forget about it
> - After the view/projection matrix has been tested