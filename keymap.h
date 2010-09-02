#ifndef _KEYMAP_H_
#define _KEYMAP_H_

#include <keycodes.h>

typedef u_int16_t keysym_t;

struct kbdiacr {
        unsigned char diacr, base, result;
};
struct kbdiacrs {
        unsigned int kb_cnt;    /* number of entries in following array */
        struct kbdiacr kbdiacr[256];    /* MAX_DIACR from keyboard.h */
};

extern keysym_t plain_map[NR_KEYS];
extern keysym_t shift_map[NR_KEYS];
extern keysym_t altgr_map[NR_KEYS];
extern keysym_t ctrl_map[NR_KEYS];
extern keysym_t shift_ctrl_map[NR_KEYS];
extern keysym_t alt_map[NR_KEYS];
extern keysym_t ctrl_alt_map[NR_KEYS];

extern keysym_t *key_maps[MAX_NR_KEYMAPS];

#endif
