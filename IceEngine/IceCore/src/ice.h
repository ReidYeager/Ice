
#ifndef ICE_H_
#define ICE_H_

#include "core/application.h"

#define ICE_ENTRYPOINT                          \
  int main()                                    \
  {                                             \
    try                                         \
    {                                           \
      Application app;                          \
      app.Run();                                \
    }                                           \
    catch (const char* e)                       \
    {                                           \
      std::cout << "Ice caught " << e << "\n";  \
    }                                           \
                                                \
    return 0;                                   \
  }                                             \

#endif // !ICE_H_
