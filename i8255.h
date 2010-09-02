#ifndef __I8255__H__
#define __I8255__H__

#include <types.h>

/* 8255 PPI drives a 8-24 keyboard controller */

/* controller commands */
#define KBDC_CMD_MOUSE_DISABLE  0xA7
#define KBDC_CMD_MOUSE_ENABLE   0xA8
#define KBDC_CMD_WRITE_MOUSE    0xD4
#define KBDC_CMD_KBD_DISABLE    0xAD    /* Keyboard interface disable */
#define KBDC_CMD_KBD_ENABLE     0xAE    /* Keyboard interface enable */
#define KBDC_CMD_READ_MODE      0x20    /* Read mode bits */
#define KBDC_CMD_WRITE_MODE     0x60    /* Write mode bits */

/* keyboard commands */
#define KBD_CMD_SET_LED         0xED
#define KBD_CMD_SELECT_SCANCODE 0xF0 /* 0: read ('C','A',?), write: 1,2,3 */
#define KBD_CMD_IDENT           0xF2 /* XT (no answer), AT: ack, MF II, ack 0xab 0x41 */
#define KBD_CMD_SET_RATE        0xF3 /* bit 0-4 rate. 5-6 pause: 250-1000 ms */

/* status register */
#define KBD_STAT_OBF            0x01
#define KBD_STAT_IBF            0x02
#define KBD_STAT_MOUSE_OBF      0x20

/* controller mode */
#define KBD_MODE_KBD_INT        0x01    /* keyboard generates irq1 */
#define KBD_MODE_MOUSE_INT      0x02    /* mouse generates irq12 */
#define KBD_MODE_SYS            0x04    /* The system flag (?) */
#define KBD_MODE_NO_KEYLOCK     0x08    /* The keylock doesn't affect the keyboard if set */
#define KBD_MODE_DISABLE_KBD    0x10    /* Disable keyboard interface */
#define KBD_MODE_DISABLE_MOUSE  0x20    /* Disable mouse interface */
#define KBD_MODE_KCC            0x40    /* Scan code conversion to PC format */
#define KBD_MODE_RFU            0x80

#define KBD_AUX_INTS_ON  (KBD_MODE_KCC | KBD_MODE_SYS | KBD_MODE_MOUSE_INT | KBD_MODE_KBD_INT)

/* mouse commands */
#define KBD_AUX_SET_RES             0xE8    /* Set resolution */
#define KBD_AUX_SET_SCALE11         0xE6    /* Set 1:1 scaling */
#define KBD_AUX_SET_SCALE21         0xE7    /* Set 2:1 scaling */
#define KBD_AUX_GET_SCALE           0xE9    /* Get scaling factor */
#define KBD_AUX_SET_STREAM          0xEA    /* Set stream mode */
#define KBD_AUX_SET_SAMPLE          0xF3    /* Set sample rate */
#define KBD_AUX_ENABLE_DEV          0xF4    /* Enable aux device */
#define KBD_AUX_DISABLE_DEV         0xF5    /* Disable aux device */
#define KBD_AUX_RESET               0xFF    /* Reset aux device */
#define KBD_AUX_ACK                 0xFA    /* Command byte ACK. */


void i8255_init();
void i8255_write_ccommand(u_int8_t cmd);
void i8255_write_command(u_int8_t cmd);
void i8255_write_data(u_int8_t cmd);
void i8255_aux_write(u_int8_t val);

void i8255_set_mode(u_int8_t val);
u_int8_t i8255_get_mode();

void i8255_set_led(int mask);

/* play sound though the PC speaker */ 
void i8255_play(int freq, int duration);

void i8255_enable_speaker(int on);
void i8255_set_freq(int freq);

#endif
