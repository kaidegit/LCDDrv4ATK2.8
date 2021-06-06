//
// Created by kai on 2021/5/16.
//

#include "lcd.h"
#include "usart.h"
#include "stdio.h"
#include "string.h"
#include "fsmc.h"

uint16_t POINT_COLOR = 0x0000;
uint16_t BACK_COLOR = 0xFFFF;

uint8_t DFT_SCAN_DIR;

lcd_dev lcddev;

void LCD_WR_REG(__IO uint16_t regval) {
    LCD->LCD_REG = regval;
}

void LCD_WR_DATA(__IO uint16_t data) {
    LCD->LCD_RAM = data;
}

uint16_t LCD_RD_DATA(void) {
    __IO uint16_t ram;
    ram = LCD->LCD_RAM;
    return ram;
}

void LCD_WriteReg(uint16_t LCD_Reg, uint16_t LCD_RegValue) {
    LCD->LCD_REG = LCD_Reg;
    LCD->LCD_RAM = LCD_RegValue;
}

uint16_t LCD_ReadReg(uint16_t LCD_Reg) {
    LCD_WR_REG(LCD_Reg);
    return LCD_RD_DATA();
}

void LCD_WriteRAM_Prepare(void) {
    LCD->LCD_REG = lcddev.wramcmd;
}

void LCD_WriteRAM(uint16_t RGB_Code) {
    LCD->LCD_RAM = RGB_Code;
}

uint16_t LCD_BGR2RGB(uint16_t c) {
    uint16_t r, g, b, rgb;
    b = (c >> 0) & 0x1f;
    g = (c >> 5) & 0x3f;
    r = (c >> 11) & 0x1f;
    rgb = (b << 11) + (g << 5) + (r << 0);
    return (rgb);
}

void LCD_Display_Dir(uint8_t dir) {
    if (dir == 0)         // 竖屏
    {
        lcddev.dir = 0;
        lcddev.width = 240;
        lcddev.height = 320;

        lcddev.wramcmd = 0X2C;
        lcddev.setxcmd = 0X2A;
        lcddev.setycmd = 0X2B;
        DFT_SCAN_DIR = U2D_R2L;
    } else {            // 横屏
        lcddev.dir = 1;
        lcddev.width = 320;
        lcddev.height = 240;

        lcddev.wramcmd = 0X2C;
        lcddev.setxcmd = 0X2A;
        lcddev.setycmd = 0X2B;
        DFT_SCAN_DIR = L2R_U2D;
    }
    LCD_Scan_Dir(DFT_SCAN_DIR);
}

void LCD_Scan_Dir(enum SCAN_DIR dir) {
    uint16_t regval = 0;
    uint16_t dirreg = 0;
    uint16_t temp;
    switch (dir) {  // 方向转换
        case 0:
            dir = 6;
            break;
        case 1:
            dir = 7;
            break;
        case 2:
            dir = 4;
            break;
        case 3:
            dir = 5;
            break;
        case 4:
            dir = 1;
            break;
        case 5:
            dir = 0;
            break;
        case 6:
            dir = 3;
            break;
        case 7:
            dir = 2;
            break;
        default:
            break;
    }
    switch (dir) {
        case L2R_U2D:// 从左到右从上到下
            regval |= (0 << 7) | (0 << 6) | (0 << 5);
            break;
        case L2R_D2U:
            regval |= (1 << 7) | (0 << 6) | (0 << 5);
            break;
        case R2L_U2D:
            regval |= (0 << 7) | (1 << 6) | (0 << 5);
            break;
        case R2L_D2U:
            regval |= (1 << 7) | (1 << 6) | (0 << 5);
            break;
        case U2D_L2R:
            regval |= (0 << 7) | (0 << 6) | (1 << 5);
            break;
        case U2D_R2L:
            regval |= (0 << 7) | (1 << 6) | (1 << 5);
            break;
        case D2U_L2R:
            regval |= (1 << 7) | (0 << 6) | (1 << 5);
            break;
        case D2U_R2L:
            regval |= (1 << 7) | (1 << 6) | (1 << 5);
            break;
        default:
            break;
    }
    dirreg = 0X36;
    regval |= 0x00;
    LCD_WriteReg(dirreg, regval);

    LCD_WR_REG(lcddev.setxcmd);
    LCD_WR_DATA(0);
    LCD_WR_DATA(0);
    LCD_WR_DATA((lcddev.width - 1) >> 8);
    LCD_WR_DATA((lcddev.width - 1) & 0XFF);
    LCD_WR_REG(lcddev.setycmd);
    LCD_WR_DATA(0);
    LCD_WR_DATA(0);
    LCD_WR_DATA((lcddev.height - 1) >> 8);
    LCD_WR_DATA((lcddev.height - 1) & 0XFF);
}

void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos) {
    LCD_WR_REG(lcddev.setxcmd);
    LCD_WR_DATA(Xpos >> 8);
    LCD_WR_DATA(Xpos & 0XFF);

    LCD_WR_REG(lcddev.setycmd);
    LCD_WR_DATA(Ypos >> 8);
    LCD_WR_DATA(Ypos & 0XFF);
}

void LCD_Clear(enum COLOR color) {
    uint32_t index = 0;
    uint32_t totalpoint = lcddev.width;
    totalpoint *= lcddev.height;
    LCD_SetCursor(0x00, 0x0000);
    LCD_WriteRAM_Prepare();
    for (index = 0; index < totalpoint; index++) {
        LCD->LCD_RAM = color;
    }
}

void LCD_Color_Fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t color) {
    uint16_t height, width;
    uint16_t i, j;
    width = ex - sx + 1;
    height = ey - sy + 1;
    for (i = 0; i < height; i++) {
        LCD_SetCursor(sx, sy + i);
        LCD_WriteRAM_Prepare();
        for (j = 0; j < width; j++)LCD->LCD_RAM = color;
    }
}

void LCD_ShowImage(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, const uint16_t *color) {
    uint16_t height, width;
    uint16_t i, j;
    width = x_end - x_start + 1;
    height = y_end - y_start + 1;
    for (i = 0; i < height; i++) {
        LCD_SetCursor(x_start, y_start + i);
        LCD_WriteRAM_Prepare();
        for (j = 0; j < width; j++)LCD->LCD_RAM = color[i * width + j];
    }
}

void LCD_DrawPoint(uint16_t x, uint16_t y) {
    LCD_SetCursor(x, y);        //设置光标位置
    LCD_WriteRAM_Prepare();    //开始写入GRAM
    LCD->LCD_RAM = POINT_COLOR;
}

void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    uint16_t t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;
    delta_x = x2 - x1;
    delta_y = y2 - y1;
    uRow = x1;
    uCol = y1;
    if (delta_x > 0) {
        incx = 1;
    } else if (delta_x == 0) {
        incx = 0;
    } else {
        incx = -1;
        delta_x = -delta_x;
    }
    if (delta_y > 0) {
        incy = 1;
    } else if (delta_y == 0) {
        incy = 0;
    } else {
        incy = -1;
        delta_y = -delta_y;
    }
    if (delta_x > delta_y) {
        distance = delta_x;
    } else {
        distance = delta_y;
    }
    for (t = 0; t <= distance + 1; t++) {
        LCD_DrawPoint(uRow, uCol);
        xerr += delta_x;
        yerr += delta_y;
        if (xerr > distance) {
            xerr -= distance;
            uRow += incx;
        }
        if (yerr > distance) {
            yerr -= distance;
            uCol += incy;
        }
    }
}

void LCD_Draw_Circle(uint16_t x0, uint16_t y0, uint8_t r) {
    int a, b;
    int di;
    a = 0;
    b = r;
    di = 3 - (r << 1);
    while (a <= b) {
        LCD_DrawPoint(x0 + a, y0 - b);
        LCD_DrawPoint(x0 + b, y0 - a);
        LCD_DrawPoint(x0 + b, y0 + a);
        LCD_DrawPoint(x0 + a, y0 + b);
        LCD_DrawPoint(x0 - a, y0 + b);
        LCD_DrawPoint(x0 - b, y0 + a);
        LCD_DrawPoint(x0 - a, y0 - b);
        LCD_DrawPoint(x0 - b, y0 - a);
        a++;
        if (di < 0) {
            di += 4 * a + 6;
        } else {
            di += 10 + 4 * (a - b);
            b--;
        }
    }
}

void LCD_Init() {
    HAL_Delay(50);
    LCD_WriteReg(0x0000, 0x0001);
    HAL_Delay(50);

    //ST7789V的id为0x8552
    LCD_WR_REG(0X04);
    lcddev.id = LCD_RD_DATA();
    lcddev.id = LCD_RD_DATA();
    lcddev.id = LCD_RD_DATA();
    lcddev.id <<= 8;
    lcddev.id |= LCD_RD_DATA();
    char ch[30];
    sprintf(ch, "lcd:%d", lcddev.id);
    HAL_UART_Transmit(&huart1, (uint8_t *) &ch, strlen(ch), 0xff);

    //---------------------------------------------------------------------------------------------------//
    LCD_WR_REG(0x11);
    HAL_Delay(120); //Delay 120ms
    //------------------------------display and color format setting--------------------------------//
    LCD_WR_REG(0x36);
    LCD_WR_DATA(0x00);
    LCD_WR_REG(0x3a);
    LCD_WR_DATA(0x05);
    //--------------------------------ST7789V Frame rate setting----------------------------------//
    LCD_WR_REG(0xb2);
    LCD_WR_DATA(0x0c);
    LCD_WR_DATA(0x0c);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x33);
    LCD_WR_DATA(0x33);
    LCD_WR_REG(0xb7);
    LCD_WR_DATA(0x35);
    //---------------------------------ST7789V Power setting--------------------------------------//
    LCD_WR_REG(0xbb);
    LCD_WR_DATA(0x28);
    LCD_WR_REG(0xc0);
    LCD_WR_DATA(0x2c);
    LCD_WR_REG(0xc2);
    LCD_WR_DATA(0x01);
    LCD_WR_REG(0xc3);
    LCD_WR_DATA(0x0b);
    LCD_WR_REG(0xc4);
    LCD_WR_DATA(0x20);
    LCD_WR_REG(0xc6);
    LCD_WR_DATA(0x0f);
    LCD_WR_REG(0xd0);
    LCD_WR_DATA(0xa4);
    LCD_WR_DATA(0xa1);
    //--------------------------------ST7789V gamma setting---------------------------------------//
    LCD_WR_REG(0xe0);
    LCD_WR_DATA(0xd0);
    LCD_WR_DATA(0x01);
    LCD_WR_DATA(0x08);
    LCD_WR_DATA(0x0f);
    LCD_WR_DATA(0x11);
    LCD_WR_DATA(0x2a);
    LCD_WR_DATA(0x36);
    LCD_WR_DATA(0x55);
    LCD_WR_DATA(0x44);
    LCD_WR_DATA(0x3a);
    LCD_WR_DATA(0x0b);
    LCD_WR_DATA(0x06);
    LCD_WR_DATA(0x11);
    LCD_WR_DATA(0x20);
    LCD_WR_REG(0xe1);
    LCD_WR_DATA(0xd0);
    LCD_WR_DATA(0x02);
    LCD_WR_DATA(0x07);
    LCD_WR_DATA(0x0a);
    LCD_WR_DATA(0x0b);
    LCD_WR_DATA(0x18);
    LCD_WR_DATA(0x34);
    LCD_WR_DATA(0x43);
    LCD_WR_DATA(0x4a);
    LCD_WR_DATA(0x2b);
    LCD_WR_DATA(0x1b);
    LCD_WR_DATA(0x1c);
    LCD_WR_DATA(0x22);
    LCD_WR_DATA(0x1f);
    LCD_WR_REG(0x29);
    LCD_WR_REG(0x2c);

    LCD_Display_Dir(0);
    HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_SET);
    LCD_Clear(WHITE);
}
