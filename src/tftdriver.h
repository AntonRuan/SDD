#ifndef TFTDRIVER_H
#define TFTDRIVER_H

#include <Arduino.h>

void tft_init();
void jpg_init();

void Web_win();
int loading();
void loaded(String ip);
void time_loop();
int anim_loop();

void set_tft_bright(int v);
void display_debug(String str);
void display_temp(String str);
void display_cpu(String str);
void weaterData(String *cityDZ, String *dataSK, String *dataFC);

void switch_screen(int index);

#endif