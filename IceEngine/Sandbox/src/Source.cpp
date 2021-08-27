
#include <iostream>
#include <ice.h>

class Application : public IceApplication
{
public:
  void ChildInit() override
  {
    DefineChildLoop(loop);

    GetMaterialIndex({ "mvp", "test" },
                     { Ice_Shader_Vert, Ice_Shader_Frag },
                     { "TestImage.png", "AltImage.png", "landscape.jpg" });

    CreateObject("Cube.obj");
  }

  void ChildShutdown() override
  {
    
  }

  void loop()
  {
    
  }
};

ICE_ENTRYPOINT;
