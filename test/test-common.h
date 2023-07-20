#ifdef _WIN32
#include "crtdbg.h"
#endif

void static inline setup_test_environment(void) {
#ifdef _WIN32
  // disable window popups
  _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
#endif 
}
