//
// Created by kai on 2021/5/17.
//

#include <cstdio>
#include <cstring>
#include "main_cpp.h"
#include "stm32f4xx_hal.h"
#include "lcd.h"
#include "touch.h"
#include "usart.h"

extern const uint8_t touxiang_map[];

void JumpToCpp() {
    main_cpp();
}

void main_cpp() {
    TP_Init();
    char ch[30];
    uint16_t x;
    uint16_t y;
    while (true) {
//        LCD_Clear(WHITE);
//        HAL_Delay(500);
//        LCD_Clear(BLACK);
//        HAL_Delay(500);
//        LCD_Clear(BLUE);
//        HAL_Delay(500);
//        LCD_Color_Fill(100,100,200,200, (uint16_t)0xFFE0);
//        HAL_Delay(1000);
//        LCD_ShowImage(10,10,130,130,(uint16_t *)touxiang_map);
//        HAL_Delay(1000);
        x = pos.x;
        y = pos.y;
        TP_Scan(0);
        if ((x != pos.x) || (y != pos.y)) {
            sprintf(ch, "x:%d,y:%d\n", pos.x, pos.y);
            HAL_UART_Transmit(&huart1, (uint8_t *) ch, strlen(ch), 0xff);
        }

    }

}