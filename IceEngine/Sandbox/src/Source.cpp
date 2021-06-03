
#include <iostream>
#include <core\application.h>

int main()
{
  printf("Sandbox\n");

  try
  {
    Application* app = new Application();
    app->Run();
  }
  catch (const char* e)
  {
    std::cout << "Ice caught " << e << "\n";
  }

  return 0;
}
