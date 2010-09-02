#ifndef _SPIN_LOCK_H_
#define _SPIN_LOCK_H_

typedef volatile int spin_lock_t;

#define SPIN_LOCK_INITIALIZER   0

#define spin_lock_init(s)       (*(s) = SPIN_LOCK_INITIALIZER)
#define spin_lock_locked(s)     (*(s) != SPIN_LOCK_INITIALIZER)
#define spin_lock_unlocked(s)   (*(s) == SPIN_LOCK_INITIALIZER)

//#define DISABLE_SPIN_LOCKS

#ifndef DISABLE_SPIN_LOCKS
#define spin_unlock(p) \
        ({  register int _u__ ; \
            __asm__ volatile("xorl %0, %0; \n\
                          xchgl %0, %1" \
                        : "=&r" (_u__), "=m" (*(p)) ); \
            0; })

#define spin_try_lock(p)\
        (!({  register int _r__; \
            __asm__ volatile("movl $1, %0; \n\
                          xchgl %0, %1" \
                        : "=&r" (_r__), "=m" (*(p)) ); \
            _r__; }))

#define spin_lock(p) \
        ({ while (!spin_try_lock(p)) while (*(p)); })

#define spin_lock_counted(p, cnt) \
        ({ int c = cnt; while (!spin_try_lock(p)) while (*(p) && cnt); })

#define spin_lock_adaptive(p) \
        spin_lock_counted(p, 0x100000)
#else

#include <libc.h>

#define spin_lock(p) xprintf("LOCKING %p\n", p);
#define spin_unlock(p) xprintf("UNLOCKING %p\n", p);
#define spin_try_lock(p) xprintf("TRY LOCKING %p\n", p);

#endif

#endif
