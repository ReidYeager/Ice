
#include <iostream>
#include <ice.h>

#include <math.h>
#include <math\vector.h>

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
      { "bluemvp", "test" },
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

    testGameObjectB.transform->position.x = 1.0f;
    testGameObject.transform->position.x = -1.0f;
    //testGameObject.transform->rotation.z = 30.0f;
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

    // TODO : (2) Push renderer information to the respective parameters
    //        As a test: send the VP matrix via only parameters
    //        As a test: send the depth image to the fragment shader

    vec4 testData = { time, sin(time), 0, 0 };
    MaterialUpdateBuffer(testMat, Ice_Shader_Vert, Ice_Shader_Buffer_Param_User1, &testData);

    vec4 blueData[2] = { { sin(time), 0.0f, 0.0f, 0.0f }, { 0.0f, glm::cos(time), 0.0f, 0.0f } };
    MaterialUpdateBuffer(blueMat, Ice_Shader_Frag, Ice_Shader_Buffer_Param_User0, &blueData[0]);
    MaterialUpdateBuffer(blueMat, Ice_Shader_Vert, Ice_Shader_Buffer_Param_User1, &blueData[1]);

  }
};

ICE_ENTRYPOINT;
