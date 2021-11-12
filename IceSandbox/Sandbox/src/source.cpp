
#include <iostream>
#include <ice.h>

#include <math.h>
#include <math\vector.h>

// TODO : De-pessimize & refactor everything

IceApplication app;

GameObject testGameObject;
GameObject testGameObjectB;
u32 testMat, blueMat;

void ChildInit()
{
  testMat = app.GetMaterialIndex(
    { "mvp", "test" },
    { Ice_Shader_Vert, Ice_Shader_Frag },
    { "TestImage.png", "AltImage.png", "landscape.jpg" });
  u32 cactusMesh = app.GetMeshIndex("BadCactus.obj");

  blueMat = app.GetMaterialIndex(
    { "bluemvp", "blue" },
    { Ice_Shader_Vert, Ice_Shader_Frag },
    { "TestImage.png", "AltImage.png", "landscape.jpg" });
  u32 cubeMesh = app.GetMeshIndex("Cube.obj");

  testGameObject = app.CreateObject();
  testGameObject.AddComponent<RenderableComponent>(cubeMesh, testMat);

  testGameObjectB = app.CreateObject();
  testGameObjectB.AddComponent<RenderableComponent>(cactusMesh, blueMat);

  testGameObjectB.transform->position.x = 1.0f;
  testGameObject.transform->position.x = -1.0f;
  //testGameObject.transform->rotation.z = 30.0f;
}

void ChildShutdown()
{
  testGameObject.Destroy();
  testGameObjectB.Destroy();
}

float runTime = 0.0f;
float smalltime = 0.0f;
int direction = 1.0f;

float r = 1.0f, g = 0.0f, b = 0.0f;
float* add, sub;
const float camMoveSpeed = 3.0f;

void ChildLoop(float _deltaTime)
{
  runTime += _deltaTime;

  //i32 x, y;
  //const float sensitivity = 0.2f;
  //Input.GetMouseDelta(&x, &y);
  //cam.Rotate({-y * sensitivity, x * sensitivity, 0});
  //cam.ClampPitch(89.0f, -89.0f);

  if (Input.IsKeyDown(Ice_Key_W))
    app.cam.position += app.cam.GetForward() * _deltaTime * camMoveSpeed;
  if (Input.IsKeyDown(Ice_Key_S))
    app.cam.position -= app.cam.GetForward() * _deltaTime * camMoveSpeed;
  if (Input.IsKeyDown(Ice_Key_D))
    app.cam.position += app.cam.GetRight() * _deltaTime * camMoveSpeed;
  if (Input.IsKeyDown(Ice_Key_A))
    app.cam.position -= app.cam.GetRight() * _deltaTime * camMoveSpeed;
  if (Input.IsKeyDown(Ice_Key_E))
    app.cam.position.y += _deltaTime * camMoveSpeed;
  if (Input.IsKeyDown(Ice_Key_Q))
    app.cam.position.y -= _deltaTime * camMoveSpeed;

  if (Input.OnKeyPressed(Ice_Key_K))
    IceLogDebug("TEST KEY PRESS ------------------------");
  if (Input.OnKeyReleased(Ice_Key_K))
    IceLogDebug("TEST KEY RELEASE ----------------------");

  if (Input.IsKeyDown(Ice_Key_Escape))
  {
    app.Close();
  }

  // TODO : ~!!~ Push renderer information to the respective parameters
  //        As a test: send the depth image to the fragment shader

  testGameObjectB.transform->rotation.y += _deltaTime * 90.0f;

  vec4 testData = { runTime, sin(runTime), 0, 0 };
  app.MaterialUpdateBuffer(testMat, Ice_Shader_Vert, Ice_Shader_Buffer_Param_User1, &testData);

  vec4 blueData[2] = { { sin(runTime), 0.0f, 0.0f, 0.0f }, { 0.0f, glm::cos(runTime), 0.0f, 0.0f } };
  app.MaterialUpdateBuffer(blueMat, Ice_Shader_Frag, Ice_Shader_Buffer_Param_User0, &blueData[0]);
  app.MaterialUpdateBuffer(blueMat, Ice_Shader_Vert, Ice_Shader_Buffer_Param_User1, &blueData[1]);
}

int main()
{
  app.Initialize(ChildInit, ChildLoop, ChildShutdown);
  app.Run();

  return 0;
}
