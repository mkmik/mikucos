#ifndef _USER_PROGRAMS_H_
#define _USER_PROGRAMS_H_

#ifdef USER_PROGRAM_LIBRARY
# define MAIN(name, signature ...) internal_ ## name (signature)
# define ONLY_INTERNAL
#else
# define MAIN(name, signature ...) main (signature)
# define ONLY_INTERNAL ()ERROR()
#endif

#endif
