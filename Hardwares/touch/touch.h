//
// Created by kai on 2021/6/3.
//

#ifndef TESTTFTFORATKF4_TOUCH_H
#define TESTTFTFORATKF4_TOUCH_H

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

struct position {
    uint16_t x;
    uint16_t y;
};

uint8_t TP_Init(void);

uint8_t TP_Scan(uint8_t tp);

extern struct position pos;

#ifdef __cplusplus
}
#endif

#endif //TESTTFTFORATKF4_TOUCH_H
