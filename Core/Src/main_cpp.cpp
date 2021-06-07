//
// Created by kai on 2021/5/17.
//

#include <cstdio>
#include <cstring>
#include "main_cpp.h"
#include "stm32f4xx_hal.h"
#include "lcd.h"
#include "../../Hardwares/touch/touch.h"
#include "usart.h"
#include "i2c.h"

extern const uint8_t touxiang_map[];

void JumpToCpp() {
    main_cpp();
}

void main_cpp() {
    LCD_Init();
    char ch[30];
    uint16_t x;
    uint16_t y;
    // E2PROM Test
//    uint8_t a[6] = {0, 1, 2, 3, 4, 5};
//    uint8_t b[6] = {123, 123, 123, 123, 123, 123};
//    HAL_I2C_Mem_Read(&hi2c1, 0xA0, 0x40, I2C_MEMADD_SIZE_8BIT, b, 6, 0xff);
//    HAL_Delay(1000);
//    sprintf(ch, "%d %d %d %d %d %d\n", b[0], b[1], b[2], b[3], b[4], b[5]);
//    HAL_UART_Transmit(&huart1, (uint8_t *) ch, strlen(ch), 0xff);
//    HAL_I2C_Mem_Write(&hi2c1, 0xA0, 0x40, I2C_MEMADD_SIZE_8BIT, a, 6, 0xff);
//    HAL_Delay(1000);
//    HAL_I2C_Mem_Read(&hi2c1, 0xA0, 0x40, I2C_MEMADD_SIZE_8BIT, b, 6, 0xff);
//    sprintf(ch, "%d %d %d %d %d %d\n", b[0], b[1], b[2], b[3], b[4], b[5]);
//    HAL_UART_Transmit(&huart1, (uint8_t *) ch, strlen(ch), 0xff);

//    TP_Init();
    while (true) {
        // LCD Test
        LCD_Clear(WHITE);
        HAL_Delay(500);
        LCD_Clear(BLACK);
        HAL_Delay(500);
        LCD_Clear(BLUE);
        HAL_Delay(500);
//        LCD_Color_Fill(100,100,200,200, (uint16_t)0xFFE0);
//        HAL_Delay(1000);
        LCD_ShowImage(0, 0, 119, 119, (uint16_t *) touxiang_map);
        HAL_Delay(1000);
//        // Touch Test
//        x = pos.x;
//        y = pos.y;
//        TP_Scan(0);
//        if ((x != pos.x) || (y != pos.y)) {
//            sprintf(ch, "x:%d,y:%d\n", pos.x, pos.y);
//            HAL_UART_Transmit(&huart1, (uint8_t *) ch, strlen(ch), 0xff);
//            LCD_DrawPoint(x, y);
//        }
    }

}