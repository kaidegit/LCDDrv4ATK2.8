//
// Created by kai on 2021/6/3.
//

#include <lcd.h>
#include <stdlib.h>
#include <math.h>
#include "touch.h"
#include "main.h"
#include "i2c.h"

struct position pos;

float xfac, yfac;
int16_t xoff, yoff;
uint8_t touchtype;
uint8_t sta;

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
            pos.x = xfac * pos.x + xoff;
            pos.y = yfac * pos.y + yoff;
        }
//        if ((sta & TP_PRES_DOWN) == 0) {
//            sta = TP_PRES_DOWN | TP_CATH_PRES;
//            x[4] = x[0];
//            y[4] = y[0];
//    }
    } else {
//        if (tp_dev.sta & TP_PRES_DOWN)//Ö®Ç°ÊÇ±»°´ÏÂµÄ
//        {
//            tp_dev.sta &= ~(1 << 7);//±ê¼Ç°´¼üËÉ¿ª
//        } else//Ö®Ç°¾ÍÃ»ÓÐ±»°´ÏÂ
//        {
//            x[4] = 0;
//            y[4] = 0;
//            x[0] = 0xffff;
//            y[0] = 0xffff;
//        }
    }
//    return tp_dev.sta & TP_PRES_DOWN;
}

#define EEPROM_ADDR 0xA0
#define SAVE_ADDR_BASE 0x28

uint8_t TP_Get_Adjdata(void) {
    uint8_t temp8;
    HAL_I2C_Mem_Read(&hi2c1, EEPROM_ADDR, SAVE_ADDR_BASE + 13, I2C_MEMADD_SIZE_8BIT, &temp8, 1, 0xff);
    if (temp8 == 0X0A) {
        int32_t temp32;
        HAL_I2C_Mem_Read(&hi2c1, EEPROM_ADDR, SAVE_ADDR_BASE, I2C_MEMADD_SIZE_8BIT, (uint8_t *) &temp32, 4, 0xff);
        xfac = temp32 / 100000000.0;
        HAL_I2C_Mem_Read(&hi2c1, EEPROM_ADDR, SAVE_ADDR_BASE + 4, I2C_MEMADD_SIZE_8BIT, (uint8_t *) &temp32, 4, 0xff);
        yfac = temp32 / 100000000.0;
        int16_t temp16;
        HAL_I2C_Mem_Read(&hi2c1, EEPROM_ADDR, SAVE_ADDR_BASE + 8, I2C_MEMADD_SIZE_8BIT, (uint8_t *) &temp16, 2, 0xff);
        xoff = temp16;
        HAL_I2C_Mem_Read(&hi2c1, EEPROM_ADDR, SAVE_ADDR_BASE + 10, I2C_MEMADD_SIZE_8BIT, (uint8_t *) &temp16, 2, 0xff);
        yoff = temp16;
        HAL_I2C_Mem_Read(&hi2c1, EEPROM_ADDR, SAVE_ADDR_BASE + 12, I2C_MEMADD_SIZE_8BIT, &temp8, 1, 0xff);
        touchtype = temp8;
        return 1;
    }
    return 0;
}

void TP_Save_Adjdata(void) {
    int32_t temp32;
    temp32 = xfac * 100000000;
    HAL_I2C_Mem_Write(&hi2c1, EEPROM_ADDR, SAVE_ADDR_BASE, I2C_MEMADD_SIZE_8BIT, (uint8_t *) &temp32, 4, 0xff);
    temp32 = yfac * 100000000;
    HAL_I2C_Mem_Write(&hi2c1, EEPROM_ADDR, SAVE_ADDR_BASE + 4, I2C_MEMADD_SIZE_8BIT, (uint8_t *) &temp32, 4, 0xff);
    HAL_I2C_Mem_Write(&hi2c1, EEPROM_ADDR, SAVE_ADDR_BASE + 8, I2C_MEMADD_SIZE_8BIT, (uint8_t *) &xoff, 2, 0xff);
    HAL_I2C_Mem_Write(&hi2c1, EEPROM_ADDR, SAVE_ADDR_BASE + 10, I2C_MEMADD_SIZE_8BIT, (uint8_t *) &yoff, 2, 0xff);
    HAL_I2C_Mem_Write(&hi2c1, EEPROM_ADDR, SAVE_ADDR_BASE + 12, I2C_MEMADD_SIZE_8BIT, (uint8_t *) &touchtype, 1, 0xff);
    uint8_t temp8 = 0X0A;
    HAL_I2C_Mem_Write(&hi2c1, EEPROM_ADDR, SAVE_ADDR_BASE + 13, I2C_MEMADD_SIZE_8BIT, (uint8_t *) &temp8, 1, 0xff);
}

void TP_Drow_Touch_Point(uint16_t x, uint16_t y, uint16_t color) {
    POINT_COLOR = color;
    LCD_DrawLine(x - 12, y, x + 13, y);
    LCD_DrawLine(x, y - 12, x, y + 13);
    LCD_DrawPoint(x + 1, y + 1);
    LCD_DrawPoint(x - 1, y + 1);
    LCD_DrawPoint(x + 1, y - 1);
    LCD_DrawPoint(x - 1, y - 1);
    LCD_Draw_Circle(x, y, 6);
}

#define TP_PRES_DOWN 0x80
#define TP_CATH_PRES 0x40

void TP_Adjust(void) {
    uint16_t pos_temp[4][2];
    uint8_t cnt = 0;
    uint16_t d1, d2;
    uint32_t tem1, tem2;
    double fac;
    uint16_t outtime = 0;
    cnt = 0;
    POINT_COLOR = BLUE;
    BACK_COLOR = WHITE;
    LCD_Clear(WHITE);
    POINT_COLOR = RED;
    LCD_Clear(WHITE);
    POINT_COLOR = BLACK;
    TP_Drow_Touch_Point(20, 20, RED);
    sta = 0;
    xfac = 0;
    while (1) {
        TP_Scan(1);
        if ((sta & 0xc0) == TP_CATH_PRES) {
            outtime = 0;
            sta &= ~(1 << 6);

            pos_temp[cnt][0] = pos.x;
            pos_temp[cnt][1] = pos.y;
            cnt++;
            switch (cnt) {
                case 1:
                    TP_Drow_Touch_Point(20, 20, WHITE);
                    TP_Drow_Touch_Point(lcddev.width - 20, 20, RED);
                    break;
                case 2:
                    TP_Drow_Touch_Point(lcddev.width - 20, 20, WHITE);
                    TP_Drow_Touch_Point(20, lcddev.height - 20, RED);
                    break;
                case 3:
                    TP_Drow_Touch_Point(20, lcddev.height - 20, WHITE);
                    TP_Drow_Touch_Point(lcddev.width - 20, lcddev.height - 20, RED);
                    break;
                case 4:

                    tem1 = abs(pos_temp[0][0] - pos_temp[1][0]);
                    tem2 = abs(pos_temp[0][1] - pos_temp[1][1]);
                    tem1 *= tem1;
                    tem2 *= tem2;
                    d1 = sqrt(tem1 + tem2);

                    tem1 = abs(pos_temp[2][0] - pos_temp[3][0]);
                    tem2 = abs(pos_temp[2][1] - pos_temp[3][1]);
                    tem1 *= tem1;
                    tem2 *= tem2;
                    d2 = sqrt(tem1 + tem2);
                    fac = (float) d1 / d2;
                    if (fac < 0.95 || fac > 1.05 || d1 == 0 || d2 == 0) {
                        cnt = 0;
                        TP_Drow_Touch_Point(lcddev.width - 20, lcddev.height - 20, WHITE);
                        TP_Drow_Touch_Point(20, 20, RED);
//                        TP_Adj_Info_Show(pos_temp[0][0], pos_temp[0][1], pos_temp[1][0], pos_temp[1][1], pos_temp[2][0],
//                                         pos_temp[2][1], pos_temp[3][0], pos_temp[3][1], fac * 100);
                        continue;
                    }
                    tem1 = abs(pos_temp[0][0] - pos_temp[2][0]);
                    tem2 = abs(pos_temp[0][1] - pos_temp[2][1]);
                    tem1 *= tem1;
                    tem2 *= tem2;
                    d1 = sqrt(tem1 + tem2);

                    tem1 = abs(pos_temp[1][0] - pos_temp[3][0]);
                    tem2 = abs(pos_temp[1][1] - pos_temp[3][1]);
                    tem1 *= tem1;
                    tem2 *= tem2;
                    d2 = sqrt(tem1 + tem2);
                    fac = (float) d1 / d2;
                    if (fac < 0.95 || fac > 1.05) {
                        cnt = 0;
                        TP_Drow_Touch_Point(lcddev.width - 20, lcddev.height - 20, WHITE);
                        TP_Drow_Touch_Point(20, 20, RED);
//                        TP_Adj_Info_Show(pos_temp[0][0], pos_temp[0][1], pos_temp[1][0], pos_temp[1][1], pos_temp[2][0],
//                                         pos_temp[2][1], pos_temp[3][0], pos_temp[3][1], fac * 100);//ÏÔÊ¾Êý¾Ý
                        continue;
                    }


                    tem1 = abs(pos_temp[1][0] - pos_temp[2][0]);
                    tem2 = abs(pos_temp[1][1] - pos_temp[2][1]);
                    tem1 *= tem1;
                    tem2 *= tem2;
                    d1 = sqrt(tem1 + tem2);

                    tem1 = abs(pos_temp[0][0] - pos_temp[3][0]);
                    tem2 = abs(pos_temp[0][1] - pos_temp[3][1]);
                    tem1 *= tem1;
                    tem2 *= tem2;
                    d2 = sqrt(tem1 + tem2);
                    fac = (float) d1 / d2;
                    if (fac < 0.95 || fac > 1.05) {
                        cnt = 0;
                        TP_Drow_Touch_Point(lcddev.width - 20, lcddev.height - 20, WHITE);
                        TP_Drow_Touch_Point(20, 20, RED);
//                        TP_Adj_Info_Show(pos_temp[0][0], pos_temp[0][1], pos_temp[1][0], pos_temp[1][1], pos_temp[2][0],
//                                         pos_temp[2][1], pos_temp[3][0], pos_temp[3][1], fac * 100);
                        continue;
                    }

                    xfac = (float) (lcddev.width - 40) / (pos_temp[1][0] - pos_temp[0][0]);
                    xoff = (lcddev.width - xfac * (pos_temp[1][0] + pos_temp[0][0])) / 2;

                    yfac = (float) (lcddev.height - 40) / (pos_temp[2][1] - pos_temp[0][1]);
                    yoff = (lcddev.height - yfac * (pos_temp[2][1] + pos_temp[0][1])) / 2;
                    if (abs(xfac) > 2 || abs(yfac) > 2)//´¥ÆÁºÍÔ¤ÉèµÄÏà·´ÁË.
                    {
                        cnt = 0;
                        TP_Drow_Touch_Point(lcddev.width - 20, lcddev.height - 20, WHITE);
                        TP_Drow_Touch_Point(20, 20, RED);
//                        LCD_ShowString(40, 26, lcddev.width, lcddev.height, 16, "TP Need readjust!");
                        touchtype = !touchtype;
                        continue;
                    }
                    POINT_COLOR = BLUE;
                    LCD_Clear(WHITE);//ÇåÆÁ
//                    LCD_ShowString(35, 110, lcddev.width, lcddev.height, 16, "Touch Screen Adjust OK!");
                    HAL_Delay(1000);
                    TP_Save_Adjdata();
                    LCD_Clear(WHITE);//ÇåÆÁ   
                    return;//Ð£ÕýÍê³É				 
            }
        }
        HAL_Delay(10);
        outtime++;
        if (outtime > 1000) {
            TP_Get_Adjdata();
            break;
        }
    }
}


uint8_t TP_Init(void) {
    TP_Read_XY();

    if (TP_Get_Adjdata()) {
        return 0;
    } else {
        LCD_Clear(WHITE);
        TP_Adjust();
        TP_Save_Adjdata();
    }
    TP_Get_Adjdata();
    return 1;
}

