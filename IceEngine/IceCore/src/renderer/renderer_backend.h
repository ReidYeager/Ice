
#ifndef RENDERER_RENDERER_BACKEND_H
#define RENDERER_RENDERER_BACKEND_H 1
// TODO : Define API agnostic calls

#include "defines.h"
#include <vulkan/vulkan.h>

class RendererBackend
{
//=================================================================================================
// VARIABLES
//=================================================================================================
private:
  //struct VulkanContext
  //{
  //  // GPU information
  //  VkDevice device;
  //} vkContext;

  VkInstance instance;

//=================================================================================================
// FUNCTIONS
//=================================================================================================
public:
  // Initializes the fundamentals required to create renderer components
  RendererBackend();
  // Destroys the fundamentals required to create renderer components
  ~RendererBackend();
  // Creates the basic set of components required to render images
  i8 CreateComponents();
  // Destroys all components
  i8 DestroyComponents();

private:
  i8 CreateInstance();

};

#endif // !RENDERER_RENDER_BACKEND_H
