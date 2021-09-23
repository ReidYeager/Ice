
#include <iostream>
#include <ice.h>

//struct dummyComponent
//{
//  int x;
//  float y;
//  char z;
//};

class Application : public IceApplication
{
public:
  void ChildInit() override
  {
    IceApplicationDefineChildLoop(loop);

    GetMaterialIndex({ "mvp", "blue" },
                     { Ice_Shader_Vert, Ice_Shader_Frag },
                     { "TestImage.png", "AltImage.png", "landscape.jpg" });

    //IceMaterial m = GetMaterialIndex({ "mvp", "test" },
    //                                 { Ice_Shader_Vert, Ice_Shader_Frag },
    //                                 { "TestImage.png", "AltImage.png", "landscape.jpg" });

    GameObject o = CreateObject("Cube.obj");

    dummyComponent& d = o.AddComponent<dummyComponent>();
    d.x = 1234;
    dummyComponent& e = o.AddComponent<dummyComponent>();

    o.RemoveComponent<dummyComponent>();
    o.RemoveComponent<dummyComponent>();

    //IceObject o = CreateObject();
    //IceRenderable r = o.AddComponent<IceRenderable>();
    //r.SetMesh("Cube.obj");
    //r.SetMaterial(m);
  }

  void ChildShutdown() override
  {
    
  }

  void loop()
  {
    
  }
};

ICE_ENTRYPOINT;
