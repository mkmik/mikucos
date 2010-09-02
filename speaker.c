#include <speaker.h>
#include <i8255.h>

void speaker_play(int freq, int duration) {
  i8255_play(freq, duration);
}

void speaker_enable(int on) {
  i8255_enable_speaker(on);
}

void speaker_set_freq(int freq) {
  i8255_set_freq(freq);
}

void speaker_on() {
  speaker_enable(1);
}

void speaker_off() {
  speaker_enable(0);
}
