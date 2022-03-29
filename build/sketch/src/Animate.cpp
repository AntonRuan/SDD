#line 1 "/Users/shuangshuangruan/Documents/Arduino/SDD/src/Animate.cpp"
#include "Animate.h"

int Animate_key = -1; //初始化图标显示帧数

#include "../img/astronaut.h"

void imgAnim(const uint8_t **Animate_value, uint32_t *Animate_size)
{
    Animate_key++;
    *Animate_value = astronaut[Animate_key];
    *Animate_size = astronaut_size[Animate_key];
    if (Animate_key >= 9)
        Animate_key = -1;
}