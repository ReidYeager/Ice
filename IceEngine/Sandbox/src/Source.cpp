
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

    testMat = GetMaterialIndex(
        { "mvp", "test" },
        { Ice_Shader_Vert, Ice_Shader_Frag },
        { "TestImage.png", "AltImage.png", "landscape.jpg" });
    u32 cactusMesh = GetMeshIndex("BadCactus.obj");

    blueMat = GetMaterialIndex(
      { "bluemvp", "blue" },
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
  float smalltime = 0.0f;
  int direction = 1.0f;

  float r = 1.0f, g = 0.0f, b = 0.0f;
  float* add, sub;

  void loop(float _deltaTime)
  {
    time += _deltaTime;

    float fragdata[4] = { glm::sin(time), 0.0f, 0.0f, 0.0f };
    float vertdata[4] = {time, glm::sin(time), 0.0f, 0.0f};

    MaterialUpdateBuffer(blueMat, vertdata, Ice_Shader_Param_User1);
    MaterialUpdateBuffer(blueMat, fragdata, Ice_Shader_Param_User0);

  }
};

ICE_ENTRYPOINT;
