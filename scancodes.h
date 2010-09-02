#ifndef _SCANCODES_H_
#define _SCANCODES_H_

/*
 * Translation of escaped scancodes to keycodes.
 * This is now user-settable.
 * The keycodes 1-88,96-111,119 are fairly standard, and
 * should probably not be changed - changing might confuse X.
 * X also interprets scancode 0x5d (KEY_Begin).
 *
 * For 1-88 keycode equals scancode.
 */

#define E0_KPENTER 96
#define E0_RCTRL   97
#define E0_KPSLASH 98
#define E0_PRSCR   99
#define E0_RALT    100
#define E0_BREAK   101  /* (control-pause) */
#define E0_HOME    102
#define E0_UP      103
#define E0_PGUP    104
#define E0_LEFT    105
#define E0_RIGHT   106
#define E0_END     107
#define E0_DOWN    108
#define E0_PGDN    109
#define E0_INS     110
#define E0_DEL     111

#define E1_PAUSE   119

/* BTC */
#define E0_MACRO   112
/* LK450 */
#define E0_F13     113
#define E0_F14     114
#define E0_HELP    115
#define E0_DO      116
#define E0_F17     117
#define E0_KPMINPLUS 118
/*
 * My OmniKey generates e0 4c for  the "OMNI" key and the
 * right alt key does nada. [kkoller@nyx10.cs.du.edu]
 */
#define E0_OK   124
/*
 * New microsoft keyboard is rumoured to have
 * e0 5b (left window button), e0 5c (right window button),
 * e0 5d (menu button). [or: LBANNER, RBANNER, RMENU]
 * [or: Windows_L, Windows_R, TaskMan]
 */
#define E0_MSLW 125
#define E0_MSRW 126
#define E0_MSTM 127

static unsigned char e0_keys[128] = {
  0, 0, 0, 0, 0, 0, 0, 0,                             /* 0x00-0x07 */
  0, 0, 0, 0, 0, 0, 0, 0,                             /* 0x08-0x0f */
  0, 0, 0, 0, 0, 0, 0, 0,                             /* 0x10-0x17 */
  0, 0, 0, 0, E0_KPENTER, E0_RCTRL, 0, 0,             /* 0x18-0x1f */
  0, 0, 0, 0, 0, 0, 0, 0,                             /* 0x20-0x27 */
  0, 0, 0, 0, 0, 0, 0, 0,                             /* 0x28-0x2f */
  0, 0, 0, 0, 0, E0_KPSLASH, 0, E0_PRSCR,             /* 0x30-0x37 */
  E0_RALT, 0, 0, 0, 0, E0_F13, E0_F14, E0_HELP,       /* 0x38-0x3f */
  E0_DO, E0_F17, 0, 0, 0, 0, E0_BREAK, E0_HOME,       /* 0x40-0x47 */
  E0_UP, E0_PGUP, 0, E0_LEFT, E0_OK, E0_RIGHT, E0_KPMINPLUS, E0_END,/* 0x48-0x4f */
  E0_DOWN, E0_PGDN, E0_INS, E0_DEL, 0, 0, 0, 0,       /* 0x50-0x57 */
  0, 0, 0, E0_MSLW, E0_MSRW, E0_MSTM, 0, 0,           /* 0x58-0x5f */
  0, 0, 0, 0, 0, 0, 0, 0,                             /* 0x60-0x67 */
  0, 0, 0, 0, 0, 0, 0, E0_MACRO,                      /* 0x68-0x6f */
  0, 0, 0, 0, 0, 0, 0, 0,                             /* 0x70-0x77 */
  0, 0, 0, 0, 0, 0, 0, 0                              /* 0x78-0x7f */
};


#endif
