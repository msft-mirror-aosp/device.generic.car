
#ifndef AUDIO_CONTROL_UTILS
#define AUDIO_CONTROL_UTILS

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
   void set_is_ducked(const char *address, bool isDucked);
#ifdef __cplusplus
}
#endif // __cplusplus
#endif // AUDIO_CONTROL_UTILS