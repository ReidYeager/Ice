# Objective
Create a temporary system that will allow the modification of models and transforms at runtime to experiment with slightly more complex scenes
> This will be replaced by a more solid system in the future
> I must spend some time to learn about setting up efficient data-structures for this task first, however.
> 
> For the meantime, this shall be used to better test the rendering processes I'd like to experiment with.

## Structures
Object
- Transform
	- vec3 -- Position
	- vec3 -- Rotation
	- vec3 -- Scale
	- Transform* -- Parent
- u32 -- materialIndex
- u32 -- meshIndex
- vector<object*> -- children




