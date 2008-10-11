
#ifndef ACE_CONFIG_H
#define ACE_CONFIG_H

#define ACE_HAS_REACTOR_NOTIFICATION_QUEUE

// Max amount of connections for non-epoll platforms
#ifndef FD_SETSIZE
  #define FD_SETSIZE 4096
#endif

#ifdef WIN32

  //disable some deprecate warnings on windows
  #ifndef _CRT_NONSTDC_NO_WARNINGS
    #define _CRT_NONSTDC_NO_WARNINGS
  #endif

  #ifndef _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
  #endif

  #ifndef _CRT_SECURE_NO_DEPRECATE
    #define _CRT_SECURE_NO_DEPRECATE
  #endif

  #ifndef _CRT_NONSTDC_NO_DEPRECATE
    #define _CRT_NONSTDC_NO_DEPRECATE
  #endif

  #ifndef _WINDOWS
    #define _WINDOWS
  #endif

  #include "ace/config-win32.h"
#else
  #error "Platform not supported"
#endif

#undef ACE_HAS_NONSTATIC_OBJECT_MANAGER

#endif  /* ACE_CONFIG_H */


