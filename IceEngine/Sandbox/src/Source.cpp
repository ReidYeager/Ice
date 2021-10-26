
#include <iostream>
#include <ice.h>

class Application : public IceApplication
{
private:
  GameObject testGameObject;
  GameObject testGameObjectB;
  u32 testMat, blueMat;

public:
  void ChildInit() override
  {
    IceApplicationDefineChildLoop(loop);

    u32 testMat = GetMaterialIndex(
        { "mvp", "test" },
        { Ice_Shader_Vert, Ice_Shader_Frag },
        { "TestImage.png", "AltImage.png", "landscape.jpg" });
    u32 cactusMesh = GetMeshIndex("BadCactus.obj");

    u32 blueMat = GetMaterialIndex(
      { "mvp", "blue" },
      { Ice_Shader_Vert, Ice_Shader_Frag },
      { "TestImage.png", "AltImage.png", "landscape.jpg" });
    u32 cubeMesh = GetMeshIndex("Cube.obj");

    testGameObject = CreateObject();
    testGameObject.AddComponent<RenderableComponent>(cubeMesh, testMat);

    testGameObjectB = CreateObject();
    testGameObjectB.AddComponent<RenderableComponent>(cactusMesh, blueMat);

    testGameObjectB.transform->position[0] = 2.0f;
    testGameObject.transform->rotation[2] = 30.0f;
  }

  void ChildShutdown() override
  {
    testGameObject.Destroy();
    testGameObjectB.Destroy();
  }

  float time = 0.0f;
  int direction = 1.0f;

  float r = 1.0f, g = 0.0f, b = 0.0f;
  float* add, sub;

  void loop(float _deltaTime)
  {
    time += _deltaTime;

    if (testGameObjectB.transform->position[1] <= -3.0f ||
        testGameObjectB.transform->position[1] >= 3.0f)
    {
      direction *= -1.0f;
    }

    testGameObjectB.transform->position[1] += direction * _deltaTime;

    testGameObjectB.transform->rotation[0] += 90.0f * _deltaTime;
    testGameObject.transform->scale[1] += 0.25f * _deltaTime;



  }
};

ICE_ENTRYPOINT;
