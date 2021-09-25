
#include <iostream>
#include <ice.h>

class Application : public IceApplication
{
private:
  GameObject testGameObject;
  GameObject testGameObjectB;

public:
  void ChildInit() override
  {
    IceApplicationDefineChildLoop(loop);

    u32 materialIndex = GetMaterialIndex(
        { "mvp", "test" },
        { Ice_Shader_Vert, Ice_Shader_Frag },
        { "TestImage.png", "AltImage.png", "landscape.jpg" });
    u32 meshIndex = GetMeshIndex("BadCactus.obj");

    u32 altMat = GetMaterialIndex(
      { "mvp", "blue" },
      { Ice_Shader_Vert, Ice_Shader_Frag },
      { "TestImage.png", "AltImage.png", "landscape.jpg" });
    u32 altMesh = GetMeshIndex("Cube.obj");

    // TODO : ---Resume--- Render objects at unique transforms

    testGameObject = CreateObject();
    testGameObject.AddComponent<RenderableComponent>(meshIndex, materialIndex);

    testGameObjectB = CreateObject();
    testGameObjectB.AddComponent<RenderableComponent>(altMesh, altMat);
  }

  void ChildShutdown() override
  {
    testGameObject.Destroy();
  }

  void loop()
  {
    
  }
};

ICE_ENTRYPOINT;
