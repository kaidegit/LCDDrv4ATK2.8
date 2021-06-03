//
// Created by kai on 2021/5/17.
//

#include "main_cpp.h"
#include "stm32f4xx_hal.h"
#include "lcd.h"

extern const uint8_t touxiang_map[];

void JumpToCpp(){
    main_cpp();
}

void main_cpp(){

    while(true){
        LCD_Clear(WHITE);
        HAL_Delay(500);
        LCD_Clear(BLACK);
        HAL_Delay(500);
        LCD_Clear(BLUE);
        HAL_Delay(500);
        LCD_Color_Fill(100,100,200,200, (uint16_t)0xFFE0);
        HAL_Delay(1000);
        LCD_ShowImage(10,10,129,129,(uint16_t *)touxiang_map);
        HAL_Delay(1000);
    }

}