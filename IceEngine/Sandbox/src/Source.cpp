
#include <iostream>
#include <ice.h>

class Application : public IceApplication
{
public:
  void ChildInit() override
  {
    DefineChildLoop(loop);
  }

  void ChildShutdown() override
  {
    
  }

  void loop()
  {
    
  }
};

ICE_ENTRYPOINT;
