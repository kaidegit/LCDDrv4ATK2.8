//
// Created by kai on 2021/6/3.
//

#include <lcd.h>
#include <stdlib.h>
#include "touch.h"
#include "main.h"

struct position pos;

const uint8_t CMD_RDX = 0XD0;
const uint8_t CMD_RDY = 0X90;

void Set_CS_High() {
    HAL_GPIO_WritePin(TOUCH_CS_GPIO_Port, TOUCH_CS_Pin, GPIO_PIN_SET);
}

void Set_CS_Low() {
    HAL_GPIO_WritePin(TOUCH_CS_GPIO_Port, TOUCH_CS_Pin, GPIO_PIN_RESET);
}

void Set_MOSI_High() {
    HAL_GPIO_WritePin(TOUCH_MOSI_GPIO_Port, TOUCH_MOSI_Pin, GPIO_PIN_SET);
}

void Set_MOSI_Low() {
    HAL_GPIO_WritePin(TOUCH_MOSI_GPIO_Port, TOUCH_MOSI_Pin, GPIO_PIN_RESET);
}

void Set_SCK_High() {
    HAL_GPIO_WritePin(TOUCH_SCK_GPIO_Port, TOUCH_SCK_Pin, GPIO_PIN_SET);
}

void Set_SCK_Low() {
    HAL_GPIO_WritePin(TOUCH_SCK_GPIO_Port, TOUCH_SCK_Pin, GPIO_PIN_RESET);
}

uint8_t Read_MISO() {
    return HAL_GPIO_ReadPin(TOUCH_MISO_GPIO_Port, TOUCH_MISO_Pin) == GPIO_PIN_SET;
}

uint8_t Read_PEN() {
    return HAL_GPIO_ReadPin(TOUCH_PEN_GPIO_Port, TOUCH_PEN_Pin) == GPIO_PIN_SET;
}

void delay_us(__IO uint32_t nTime) {
    int old_val, new_val, val;

    if (nTime > 900) {
        for (old_val = 0; old_val < nTime / 900; old_val++) {
            delay_us(900);
        }
        nTime = nTime % 900;
    }

    old_val = SysTick->VAL;
    new_val = old_val - HAL_RCC_GetHCLKFreq() / 1000000 * nTime;
    if (new_val >= 0) {
        do {
            val = SysTick->VAL;
        } while ((val < old_val) && (val >= new_val));
    } else {
        new_val += HAL_RCC_GetHCLKFreq() / 1000000 * 1000;
        do {
            val = SysTick->VAL;
        } while ((val <= old_val) || (val > new_val));
    }
}

void SPI_WriteByte(uint8_t Byte) {
    for (uint8_t i = 0; i < 8; i++) {
        if (Byte & 0x80) {
            Set_MOSI_High();
        } else {
            Set_MOSI_Low();
        }
        Byte <<= 1;
        Set_SCK_Low();
        delay_us(1);
        Set_SCK_High();
    }
}

uint16_t TP_Read_AD(uint8_t CMD) {
    uint8_t count = 0;
    uint16_t Num = 0;
    Set_CS_Low();
    Set_SCK_Low();
    Set_MOSI_Low();
    SPI_WriteByte(CMD);
    delay_us(6);
    Set_SCK_Low();
    delay_us(1);
    Set_SCK_High();
    delay_us(1);
    Set_SCK_Low();
    for (count = 0; count < 16; count++) {
        Num <<= 1;
        Set_SCK_Low();
        delay_us(1);
        Set_SCK_High();
        if (Read_MISO()) {
            Num++;
        }
    }
    Num >>= 4;
    Set_CS_High();
    return (Num);
}

#define READ_TIMES 5
#define LOST_VAL 1

int cmp(const void *a, const void *b) {
    return *(uint16_t *) a - *(uint16_t *) b;
}

uint16_t TP_Read_XOrY(uint8_t xOry) {
    if ((xOry != CMD_RDX) && (xOry != CMD_RDY)) {
        return 0;
    }
    uint16_t i;
    uint16_t buf[READ_TIMES];
    uint16_t sum = 0;
    uint16_t temp;

    for (i = 0; i < READ_TIMES; i++) {
        buf[i] = TP_Read_AD(xOry);
    }

    qsort(buf, READ_TIMES, sizeof(uint16_t), cmp);

    sum = 0;
    for (i = LOST_VAL; i < READ_TIMES - LOST_VAL; i++) {
        sum += buf[i];
    }
    temp = sum / (READ_TIMES - 2 * LOST_VAL);
    return temp;
}

uint8_t TP_Read_XY() {
    uint16_t x_temp, y_temp;
    x_temp = TP_Read_XOrY(CMD_RDX);
    y_temp = TP_Read_XOrY(CMD_RDY);
    pos.x = x_temp;
    pos.y = y_temp;
    return 1;
}

#define ERR_RANGE 50

uint8_t TP_Read_XY_Twice() {
    uint16_t x1, y1;
    uint16_t x2, y2;
    uint8_t flag;
    flag = TP_Read_XY();
    if (flag == 0) {
        return 0;
    }
    x1 = pos.x;
    y1 = pos.y;
    flag = TP_Read_XY();
    if (flag == 0) {
        return 0;
    }
    x2 = pos.x;
    y2 = pos.y;
    if (((x2 <= x1 && x1 < x2 + ERR_RANGE) || (x1 <= x2 && x2 < x1 + ERR_RANGE))
        && ((y2 <= y1 && y1 < y2 + ERR_RANGE) || (y1 <= y2 && y2 < y1 + ERR_RANGE))) {
        pos.x = (x1 + x2) / 2;
        pos.y = (y1 + y2) / 2;
        return 1;
    } else {
        return 0;
    }
}

uint8_t TP_Scan(uint8_t tp) {
    if (Read_PEN() == 0) {
        if (tp) {
            TP_Read_XY_Twice();
        } else if (TP_Read_XY_Twice()) {
//            tp_dev.x[0] = tp_dev.xfac * tp_dev.x[0] + tp_dev.xoff;
//            tp_dev.y[0] = tp_dev.yfac * tp_dev.y[0] + tp_dev.yoff;
        }
//        if ((tp_dev.sta & TP_PRES_DOWN) == 0) {
//            tp_dev.sta = TP_PRES_DOWN | TP_CATH_PRES;
//            tp_dev.x[4] = tp_dev.x[0];
//            tp_dev.y[4] = tp_dev.y[0];
//    }
    } else {
//        if (tp_dev.sta & TP_PRES_DOWN)//Ö®Ç°ÊÇ±»°´ÏÂµÄ
//        {
//            tp_dev.sta &= ~(1 << 7);//±ê¼Ç°´¼üËÉ¿ª
//        } else//Ö®Ç°¾ÍÃ»ÓÐ±»°´ÏÂ
//        {
//            tp_dev.x[4] = 0;
//            tp_dev.y[4] = 0;
//            tp_dev.x[0] = 0xffff;
//            tp_dev.y[0] = 0xffff;
//        }
    }
//    return tp_dev.sta & TP_PRES_DOWN;
}

uint8_t TP_Init(void) {
    TP_Read_XY();
//    AT24CXX_Init();
//    if (TP_Get_Adjdata())return 0;
//    else {
//        LCD_Clear(WHITE);
//        TP_Adjust();
//        TP_Save_Adjdata();
//    }
//    TP_Get_Adjdata();
    return 1;
}

