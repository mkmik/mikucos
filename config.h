#ifndef __CONFIG_H__
#define __CONFIG_H__

/** play a note while pressing a key */
#ifndef KEYSOUND
#define KEYSOUND 1
#endif

/** startup threading during init().
 * if enabled main will be running in a thread */
#ifndef STARTUP_THREADING
#define STARTUP_THREADING 1
#endif

/** enable pentium III specific features */
#ifndef CONFIG_MPENTIUMIII
#define CONFIG_MPENTIUMIII 0 
#endif

/** initialze and use serial port IO */
#ifndef USE_UART
#define USE_UART 1
#endif

/** debug malloc and free */
#ifndef MALLOC_DEBUGGER
#define MALLOC_DEBUGGER 0
#endif

/** use mal from acaros */
#ifndef USE_MAL
#define USE_MAL 0
#endif

// hardcode
#define LITTLE_ENDIAN 0
#define BIG_ENDIAN 1
#define ENDIANESS LITTLE_ENDIAN

#endif
