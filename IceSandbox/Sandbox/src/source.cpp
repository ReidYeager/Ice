
#include <stdio.h>
#include <ice.h>

void Init(struct IceClient* _app)
{
  printf("Game Init\n");
}

u32 count = 0;

void Loop(struct IceClient* _app, f32 _deltaTime)
{
  count++;
}

void Shutdown(struct IceClient* _app)
{
  printf("Game Shutdown -- %u\n", count);
}

b8 IceInitializeClient(IceClient* _outClient)
{
  _outClient->Init = Init;
  _outClient->Update = Loop;
  _outClient->Shutdown = Shutdown;

  return true;
}
