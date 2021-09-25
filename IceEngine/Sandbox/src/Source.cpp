
#include <iostream>
#include <ice.h>

struct dummyComponent
{
  int x;
  float y;
  char z;
};

class Application : public IceApplication
{
private:
  GameObject testGameObject;

public:
  void ChildInit() override
  {
    IceApplicationDefineChildLoop(loop);

    testGameObject = CreateObject();

    u32 materialIndex = GetMaterialIndex(
        { "mvp", "test" },
        { Ice_Shader_Vert, Ice_Shader_Frag },
        { "TestImage.png", "AltImage.png", "landscape.jpg" });
    u32 meshIndex = GetMeshIndex("Sphere.obj");

    testGameObject = CreateObject();

    testGameObject.AddComponent<RenderableComponent>(meshIndex, materialIndex);
  }

  void ChildShutdown() override
  {
    
  }

  void loop()
  {
    
  }
};

ICE_ENTRYPOINT;
