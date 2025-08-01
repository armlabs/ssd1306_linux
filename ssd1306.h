#ifndef __SSD1306_H__
#define __SSD1306_H__

#include <stdbool.h>

#define SSD1306_MAX_WIDTH 128
#define SSD1306_MAX_HEIGHT 64

#define SSD1306_I2C_ADDR            0x3c

#define SSD1306_COMM_CONTROL_BYTE   0x00
#define SSD1306_DATA_CONTROL_BYTE   0x40

#define SSD1306_COMM_DISPLAY_OFF    0xae
#define SSD1306_COMM_DISPLAY_ON     0xaf
#define SSD1306_COMM_HORIZ_NORM     0xa0
#define SSD1306_COMM_HORIZ_FLIP     0xa1
#define SSD1306_COMM_RESUME_RAM     0xa4
#define SSD1306_COMM_IGNORE_RAM     0xa5
#define SSD1306_COMM_DISP_NORM      0xa6
#define SSD1306_COMM_DISP_INVERSE   0xa7
#define SSD1306_COMM_MULTIPLEX      0xa8
#define SSD1306_COMM_VERT_OFFSET    0xd3
#define SSD1306_COMM_CLK_SET        0xd5
#define SSD1306_COMM_PRECHARGE      0xd9
#define SSD1306_COMM_COM_PIN        0xda
#define SSD1306_COMM_DESELECT_LV    0xdb
#define SSD1306_COMM_CONTRAST       0x81
#define SSD1306_COMM_DISABLE_SCROLL 0x2e
#define SSD1306_COMM_ENABLE_SCROLL  0x2f
#define SSD1306_COMM_PAGE_NUMBER    0xb0
#define SSD1306_COMM_LOW_COLUMN     0x00
#define SSD1306_COMM_HIGH_COLUMN    0x10

#define SSD1306_COMM_START_LINE     0x40

#define SSD1306_COMM_CHARGE_PUMP    0x8d

#define SSD1306_COMM_SCAN_NORM      0xc0
#define SSD1306_COMM_SCAN_REVS      0xc8

#define SSD1306_COMM_MEMORY_MODE    0x20
#define SSD1306_COMM_SET_COL_ADDR   0x21
#define SSD1306_COMM_SET_PAGE_ADDR  0x22

#define SSD1306_HORI_MODE           0x00
#define SSD1306_VERT_MODE           0x01
#define SSD1306_PAGE_MODE           0x02

#define SSD1306_FONT_SMALL          0x00
#define SSD1306_FONT_NORMAL         0x01

#define SSD1306_128_64_LINES        64
#define SSD1306_128_32_LINES        32
#define SSD1306_64_48_LINES         48

#define SSD1306_128_64_COLUMNS      128
#define SSD1306_128_32_COLUMNS      128
#define SSD1306_64_48_COLUMNS       64


uint8_t ssd1306_init(uint8_t i2c_dev);
uint8_t ssd1306_end();
uint8_t ssd1306_oled_onoff(uint8_t onoff);
uint8_t ssd1306_oled_horizontal_flip(uint8_t flip);
uint8_t ssd1306_oled_display_flip(uint8_t flip);
uint8_t ssd1306_oled_multiplex(uint8_t row);
uint8_t ssd1306_oled_vert_shift(uint8_t offset);
uint8_t ssd1306_oled_set_clock(uint8_t clk);
uint8_t ssd1306_oled_set_precharge(uint8_t precharge);
uint8_t ssd1306_oled_set_deselect(uint8_t voltage);
uint8_t ssd1306_oled_set_com_pin(uint8_t value);
uint8_t ssd1306_oled_set_mem_mode(uint8_t mode);
uint8_t ssd1306_oled_set_col(uint8_t start, uint8_t end);
uint8_t ssd1306_oled_set_page(uint8_t start, uint8_t end);
uint8_t ssd1306_oled_set_constrast(uint8_t value);
uint8_t ssd1306_oled_scroll_onoff(uint8_t onoff);
uint8_t ssd1306_oled_set_X(uint8_t x);
uint8_t ssd1306_oled_set_Y(uint8_t y);
uint8_t ssd1306_oled_set_XY(uint8_t x, uint8_t y);
uint8_t ssd1306_oled_set_rotate(uint8_t degree);
uint8_t ssd1306_oled_default_config(uint8_t oled_lines, uint8_t oled_columns);
uint8_t ssd1306_oled_write_line(uint8_t size, char* ptr);
uint8_t ssd1306_oled_write_string(uint8_t size, char* ptr);
uint8_t ssd1306_oled_clear_line(uint8_t row);
uint8_t ssd1306_oled_clear_screen();
uint8_t ssd1306_oled_save_resolution(uint8_t column, uint8_t row);
uint8_t ssd1306_oled_load_resolution();
uint8_t ssd1306_oled_draw_bitmap(bool bitmap[SSD1306_MAX_HEIGHT][SSD1306_MAX_WIDTH]);

#endif
