# External engine API
1. Initialize & shutdown
2. Define client's Initialize, Loop, and Shutdown functions
3. Create/destroy windows
4. Create/destroy game objects
	1. Add/remove components
5. Define systems that modify object component values
6. Create materials

## Probably a good idea:
- Memory management access
- File loading/writing

---
# Layers
---

# Platform layer
1. Create/destroy new windows
2. Memory functions
	1. Allocate memory
	2. Free memory
3. Create the components required to present images to the window
4. Filesystem access

## ECS layer
1. Create/destroy entity
2. Add component to entity

> Just use entt for the time being.
> Set up some abstractions to be able to easily replace it at a later time.

## Render layer
1. Initialize/shutdown the selected rendering API
	- Should only have one rendering API active at a time
2. Render meshes
	- Should this layer handle mesh storage?
3. Render images
4. Manage materials

###  Core layer
1. Asset management
	1. Meshes
	2. Images
	3. ~~Audio~~
2. Memory management
