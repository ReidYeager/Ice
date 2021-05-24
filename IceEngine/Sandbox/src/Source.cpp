
#include <iostream>
#include <core\application.h>

int main()
{
  printf("Sandbox\n");

  Application* app = new Application();
  app->Run();

  return 0;
}
