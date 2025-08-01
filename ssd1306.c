#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "linux_i2c.h"
#include "ssd1306.h"
#include "font.h"

const char init_oled_type_file[] = "/tmp/.ssd1306_oled_type";

static uint8_t data_buf[1024];
static uint8_t max_lines = 0;
static uint8_t max_columns = 0;
static uint8_t global_x = 0;
static uint8_t global_y = 0;

uint8_t ssd1306_init(uint8_t i2c_dev)
{
    uint8_t rc;
    rc = _i2c_init(i2c_dev, SSD1306_I2C_ADDR);
    if (rc > 0)
        return rc;
        
    // test i2c connection
    uint8_t cmd = SSD1306_COMM_CONTROL_BYTE;
    uint8_t result = 0;
    _i2c_write(&cmd, 1);
    _i2c_read(&result, 1);
    if (result == 0)
        return 1;
    
    return 0;
}

uint8_t ssd1306_end()
{
    return _i2c_close();
}

uint8_t ssd1306_oled_onoff(uint8_t onoff)
{
    data_buf[0] = SSD1306_COMM_CONTROL_BYTE;
    if (onoff == 0)
        data_buf[1] = SSD1306_COMM_DISPLAY_OFF;
    else
        data_buf[1] = SSD1306_COMM_DISPLAY_ON;
    
    return _i2c_write(data_buf, 2);
}

uint8_t ssd1306_oled_horizontal_flip(uint8_t flip)
{
    data_buf[0] = SSD1306_COMM_CONTROL_BYTE;
    if (flip == 0)
        data_buf[1] = SSD1306_COMM_HORIZ_NORM;
    else
        data_buf[1] = SSD1306_COMM_HORIZ_FLIP;
    
    return _i2c_write(data_buf, 2);
}

uint8_t ssd1306_oled_display_flip(uint8_t flip)
{
    data_buf[0] = SSD1306_COMM_CONTROL_BYTE;
    if (flip == 0)
        data_buf[1] = SSD1306_COMM_DISP_NORM;
    else
        data_buf[1] = SSD1306_COMM_DISP_INVERSE;
    
    return _i2c_write(data_buf, 2);
}

// 128x32 please use value 32
// 128x64 please use value 64
uint8_t ssd1306_oled_multiplex(uint8_t row)
{
    data_buf[0] = SSD1306_COMM_CONTROL_BYTE;
    data_buf[1] = SSD1306_COMM_MULTIPLEX;
    data_buf[2] = row - 1;
    
    return _i2c_write(data_buf, 3);
}

// offset range 0x00~0x3f
uint8_t ssd1306_oled_vert_shift(uint8_t offset)
{
    data_buf[0] = SSD1306_COMM_CONTROL_BYTE;
    data_buf[1] = SSD1306_COMM_VERT_OFFSET;
    data_buf[2] = offset;
    
    return _i2c_write(data_buf, 3);
}

// default value for clk is 0x80
uint8_t ssd1306_oled_set_clock(uint8_t clk)
{
    data_buf[0] = SSD1306_COMM_CONTROL_BYTE;
    data_buf[1] = SSD1306_COMM_CLK_SET;
    data_buf[2] = clk;
    
    return _i2c_write(data_buf, 3);
}

// default value for precharge is 0xf1
uint8_t ssd1306_oled_set_precharge(uint8_t precharge)
{
    data_buf[0] = SSD1306_COMM_CONTROL_BYTE;
    data_buf[1] = SSD1306_COMM_PRECHARGE;
    data_buf[2] = precharge;
    
    return _i2c_write(data_buf, 3);
}

// default value for deselect is 0x40
uint8_t ssd1306_oled_set_deselect(uint8_t voltage)
{
    data_buf[0] = SSD1306_COMM_CONTROL_BYTE;
    data_buf[1] = SSD1306_COMM_DESELECT_LV;
    data_buf[2] = voltage;
    
    return _i2c_write(data_buf, 3);
}

// default value for com pin is 0x02
uint8_t ssd1306_oled_set_com_pin(uint8_t value)
{
    data_buf[0] = SSD1306_COMM_CONTROL_BYTE;
    data_buf[1] = SSD1306_COMM_COM_PIN;
    data_buf[2] = value;
    
    return _i2c_write(data_buf, 3);
}

// default value use SSD1306_PAGE_MODE
uint8_t ssd1306_oled_set_mem_mode(uint8_t mode)
{
    data_buf[0] = SSD1306_COMM_CONTROL_BYTE;
    data_buf[1] = SSD1306_COMM_MEMORY_MODE;
    data_buf[2] = mode;
    
    return _i2c_write(data_buf, 3);
}

uint8_t ssd1306_oled_set_col(uint8_t start, uint8_t end)
{
    data_buf[0] = SSD1306_COMM_CONTROL_BYTE;
    data_buf[1] = SSD1306_COMM_SET_COL_ADDR;
    data_buf[2] = start;
    data_buf[3] = end;
    
    return _i2c_write(data_buf, 4);
}

uint8_t ssd1306_oled_set_page(uint8_t start, uint8_t end)
{
    data_buf[0] = SSD1306_COMM_CONTROL_BYTE;
    data_buf[1] = SSD1306_COMM_SET_PAGE_ADDR;
    data_buf[2] = start;
    data_buf[3] = end;
    
    return _i2c_write(data_buf, 4);
}

// default contrast value is 0x7f
uint8_t ssd1306_oled_set_constrast(uint8_t value)
{
    data_buf[0] = SSD1306_COMM_CONTROL_BYTE;
    data_buf[1] = SSD1306_COMM_CONTRAST;
    data_buf[2] = value;
    
    return _i2c_write(data_buf, 3);
}

uint8_t ssd1306_oled_scroll_onoff(uint8_t onoff)
{
    data_buf[0] = SSD1306_COMM_CONTROL_BYTE;
    if (onoff == 0)
        data_buf[1] = SSD1306_COMM_DISABLE_SCROLL;
    else
        data_buf[1] = SSD1306_COMM_ENABLE_SCROLL;
    
    return _i2c_write(data_buf, 2);
}

uint8_t ssd1306_oled_set_X(uint8_t x)
{
    if (x >= max_columns)
        return 1;

    global_x = x;
    
    data_buf[0] = SSD1306_COMM_CONTROL_BYTE;
    data_buf[1] = SSD1306_COMM_LOW_COLUMN | (x & 0x0f);
    data_buf[2] = SSD1306_COMM_HIGH_COLUMN | ((x >> 4) & 0x0f);
    
    return _i2c_write(data_buf, 3);
}

uint8_t ssd1306_oled_set_Y(uint8_t y)
{
    if (y >= (max_lines / 8))
        return 1;

    global_y = y;
    
    data_buf[0] = SSD1306_COMM_CONTROL_BYTE;
    data_buf[1] = SSD1306_COMM_PAGE_NUMBER | (y & 0x0f);

    return _i2c_write(data_buf, 2);
}

uint8_t ssd1306_oled_set_XY(uint8_t x, uint8_t y)
{
    if (x >= max_columns || y >= (max_lines / 8))
        return 1;

    global_x = x;
    global_y = y;
    
    data_buf[0] = SSD1306_COMM_CONTROL_BYTE;
    data_buf[1] = SSD1306_COMM_PAGE_NUMBER | (y & 0x0f);

    data_buf[2] = SSD1306_COMM_LOW_COLUMN | (x & 0x0f);
    
    data_buf[3] = SSD1306_COMM_HIGH_COLUMN | ((x >> 4) & 0x0f);
    
    return _i2c_write(data_buf, 4);
}

uint8_t ssd1306_oled_set_rotate(uint8_t degree)
{
    // only degree 0 and 180
    if (degree == 0)
    {
        data_buf[0] = SSD1306_COMM_CONTROL_BYTE;
        data_buf[1] = SSD1306_COMM_HORIZ_FLIP;
        data_buf[2] = SSD1306_COMM_SCAN_REVS;
    
        return _i2c_write(data_buf, 3);
    }
    else if (degree == 180)
    {
        data_buf[0] = SSD1306_COMM_CONTROL_BYTE;
        data_buf[1] = SSD1306_COMM_HORIZ_NORM;
        data_buf[2] = SSD1306_COMM_SCAN_NORM;
    
        return _i2c_write(data_buf, 3);
    }
    else
        return 1;
}

uint8_t ssd1306_oled_default_config(uint8_t oled_lines, uint8_t oled_columns)
{
    if (oled_lines != SSD1306_128_64_LINES && oled_lines != SSD1306_128_32_LINES && SSD1306_64_48_LINES)
        oled_lines = SSD1306_128_64_LINES;
        
    if (oled_columns != SSD1306_128_64_COLUMNS && oled_lines != SSD1306_128_32_COLUMNS && SSD1306_64_48_COLUMNS)
        oled_columns = SSD1306_128_64_COLUMNS;
        
    max_lines = oled_lines;
    max_columns = oled_columns;
    global_x = 0;
    global_y = 0;
    
    if (ssd1306_oled_save_resolution(max_columns, max_lines) != 0)
        return 1;
    
    uint16_t i = 0;
    data_buf[i++] = SSD1306_COMM_CONTROL_BYTE;  //command control byte
    data_buf[i++] = SSD1306_COMM_DISPLAY_OFF;   //display off
    data_buf[i++] = SSD1306_COMM_DISP_NORM;     //Set Normal Display (default)
    data_buf[i++] = SSD1306_COMM_CLK_SET;       //SETDISPLAYCLOCKDIV
    data_buf[i++] = 0x80;                       // the suggested ratio 0x80
    data_buf[i++] = SSD1306_COMM_MULTIPLEX;     //SSD1306_SETMULTIPLEX
    data_buf[i++] = oled_lines - 1;             // height is 32 or 64 (always -1)
    data_buf[i++] = SSD1306_COMM_VERT_OFFSET;   //SETDISPLAYOFFSET
    data_buf[i++] = 0;                          //no offset
    data_buf[i++] = SSD1306_COMM_START_LINE;    //SETSTARTLINE
    data_buf[i++] = SSD1306_COMM_CHARGE_PUMP;   //CHARGEPUMP
    data_buf[i++] = 0x14;                       //turn on charge pump
    data_buf[i++] = SSD1306_COMM_MEMORY_MODE;   //MEMORYMODE
    data_buf[i++] = SSD1306_PAGE_MODE;          // page mode
    data_buf[i++] = SSD1306_COMM_HORIZ_NORM;    //SEGREMAP  Mirror screen horizontally (A0)
    data_buf[i++] = SSD1306_COMM_SCAN_NORM;     //COMSCANDEC Rotate screen vertically (C0)
    data_buf[i++] = SSD1306_COMM_COM_PIN;       //HARDWARE PIN 
    if (oled_lines == 32)
        data_buf[i++] = 0x02;                       // for 32 lines
    else
        data_buf[i++] = 0x12;                       // for 64 lines or 48 lines
    data_buf[i++] = SSD1306_COMM_CONTRAST;      //SETCONTRAST
    data_buf[i++] = 0x7f;                       // default contract value
    data_buf[i++] = SSD1306_COMM_PRECHARGE;     //SETPRECHARGE
    data_buf[i++] = 0xf1;                       // default precharge value
    data_buf[i++] = SSD1306_COMM_DESELECT_LV;   //SETVCOMDETECT                
    data_buf[i++] = 0x40;                       // default deselect value
    data_buf[i++] = SSD1306_COMM_RESUME_RAM;    //DISPLAYALLON_RESUME
    data_buf[i++] = SSD1306_COMM_DISP_NORM;     //NORMALDISPLAY
    data_buf[i++] = SSD1306_COMM_DISPLAY_ON;    //DISPLAY ON             
    data_buf[i++] = SSD1306_COMM_DISABLE_SCROLL;//Stop scroll
    
    return _i2c_write(data_buf, i);
}

uint8_t ssd1306_oled_write_line(uint8_t size, char* ptr)
{
    uint16_t i = 0;
    uint16_t index = 0;
    uint8_t* font_table = 0;
    uint8_t font_table_width = 0;
    
    if (ptr == 0)
        return 1;
    
    if (size == SSD1306_FONT_SMALL) // 5x7
    {
        font_table = (uint8_t*)font5x7;
        font_table_width = 5;
    }
    else if (size == SSD1306_FONT_NORMAL) // 8x8
    {
        font_table = (uint8_t*)font8x8;
        font_table_width = 8;
    }
    else
        return 1;
    
    data_buf[i++] = SSD1306_DATA_CONTROL_BYTE;
    
    // font table range in ascii table is from 0x20(space) to 0x7e(~)
    while (ptr[index] != 0 && i <= 1024)
    {
        if ((ptr[index] < ' ') || (ptr[index] > '~'))
            return 1;

        uint8_t* font_ptr = &font_table[(ptr[index] - 0x20) * font_table_width];
        uint8_t j = 0;
        for (j = 0; j < font_table_width; j++)
        {
            data_buf[i++] = font_ptr[j];
            if (i > 1024)
                return 1;
        }
        // insert 1 col space for small font size)
        if (size == SSD1306_FONT_SMALL)
            data_buf[i++] = 0x00;
        index++;
    }
    
    return _i2c_write(data_buf, i);
}

uint8_t ssd1306_oled_write_string(uint8_t size, char* ptr)
{
    uint8_t rc = 0;
    
    if (ptr == 0)
        return 1;
    
    char* line = 0;
    char* cr = 0;
    char buf[20];
    
    line = ptr;
    do {
        memset(buf, 0, 20);
        cr = strstr(line, "\\n");
        if (cr != NULL)
        {
            strncpy(buf, line, cr - line);
        }
        else
        {
            strcpy(buf, line);
        }
        
        // set cursor position
        ssd1306_oled_set_XY(global_x, global_y);
        rc += ssd1306_oled_write_line(size, buf);
        
        if (cr != NULL)
        {
            line = &cr[2];
            global_x = 0;
            global_y++;
            if (global_y >= (max_lines / 8))
                global_y = 0;
        }
        else
            line = NULL;
                
    }while (line != NULL);
    
    return rc;
}

uint8_t ssd1306_oled_clear_line(uint8_t row)
{
    uint8_t i;
    if (row >= (max_lines / 8))
        return 1;
        
    ssd1306_oled_set_XY(0, row);
    data_buf[0] = SSD1306_DATA_CONTROL_BYTE;
    for (i = 0; i < max_columns; i++)
        data_buf[i+1] = 0x00;
        
    return _i2c_write(data_buf, 1 + max_columns);
}

uint8_t ssd1306_oled_clear_screen()
{
    uint8_t rc = 0;
    uint8_t i;
    
    for (i = 0; i < (max_lines / 8); i++)
    {
        rc += ssd1306_oled_clear_line(i);
    }
    
    return rc;
}

uint8_t ssd1306_oled_save_resolution(uint8_t column, uint8_t row)
{
    FILE* fp;
    
    fp = fopen(init_oled_type_file, "w");
    
    if (fp == NULL)
    {
        // file create failed
        return 1;
    }
    
    fprintf(fp, "%hhux%hhu", column, row);
    fclose(fp);
    
    return 0;
}

uint8_t ssd1306_oled_load_resolution()
{
    FILE* fp;
    
    fp = fopen(init_oled_type_file, "r");
    
    if (fp == NULL)
    {
        // file not exists
        return 1;
    }
    
    // file exists
    fscanf(fp, "%hhux%hhu", &max_columns, &max_lines);
    fclose(fp);
    
    return 0;
}

uint8_t ssd1306_oled_draw_bitmap(bool bitmap[SSD1306_MAX_HEIGHT][SSD1306_MAX_WIDTH])
{
    if (max_lines == 0 || max_columns == 0)
        return 1;

    uint8_t pages = max_lines / 8; // 8 pixels per page
    uint8_t col, page;

    for (page = 0; page < pages; page++)
    {
        // Set page and column to start of the line
        ssd1306_oled_set_XY(0, page);

        // First byte is the control byte
        data_buf[0] = SSD1306_DATA_CONTROL_BYTE;

        for (col = 0; col < max_columns; col++)
        {
            uint8_t byte = 0;
            for (uint8_t bit = 0; bit < 8; bit++)
            {
                if (bitmap[page * 8 + bit][col])
                    byte |= (1 << bit);
            }
            data_buf[col + 1] = byte;
        }

        // Send the full line (control byte + display data)
        if (_i2c_write(data_buf, max_columns + 1) != 0)
            return 1;
    }

    return 0;
}
