#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include "ssd1306.h"

void print_help()
{
    printf("help message\n\n");
    printf("-I\t\tinit oled (128x32 or 128x64 or 64x48)\n");
    printf("-c\t\tclear (line number or all)\n");
    printf("-d\t\t0/display off 1/display on\n");
    printf("-f\t\t0/small font 5x7 1/normal font 8x8 (default normal font)\n");
    printf("-h\t\thelp message\n");
    printf("-i\t\t0/normal oled 1/invert oled\n");
    printf("-b\t\tread and display a bmp file (128x64 or 128x32 @ 1bpp)\n");
    printf("-l\t\tput your line to display\n");
    printf("-m\t\tput your strings to oled\n");
    printf("-n\t\tI2C device node address (0,1,2..., default 0)\n");
    printf("-r\t\t0/normal 180/rotate\n");
    printf("-x\t\tx position\n");
    printf("-y\t\ty position\n");
}

// New buffer and path for bitmap file
#define MAX_WIDTH 128
#define MAX_HEIGHT 64
bool bitmap[MAX_HEIGHT][MAX_WIDTH];
char bmp_path[256] = {0};

int load_bitmap_to_bool_array(const char *filepath, bool output[MAX_HEIGHT][MAX_WIDTH], int *out_width, int *out_height) {
    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        perror("fopen");
        return -1;
    }

    uint8_t header[54];
    if (fread(header, 1, 54, fp) != 54 || header[0] != 'B' || header[1] != 'M') {
        fclose(fp);
        fprintf(stderr, "Invalid BMP file\n");
        return -1;
    }

    uint32_t offset = *(uint32_t *)&header[10];
    int width = *(int32_t *)&header[18];
    int height = *(int32_t *)&header[22];
    uint16_t bpp = *(uint16_t *)&header[28];

    if (bpp != 1 || width > MAX_WIDTH || (height != 32 && height != 64)) {
        fclose(fp);
        fprintf(stderr, "Only 1bpp 128x32 or 128x64 BMPs supported.\n");
        return -1;
    }

    fseek(fp, offset, SEEK_SET);
    size_t row_bytes = ((width + 31) / 32) * 4;

    memset(output, 0, sizeof(bool) * MAX_WIDTH * MAX_HEIGHT);

    // BMP rows are bottom-up
    for (int row = height - 1; row >= 0; row--) {
        uint8_t row_data[row_bytes];
        fread(row_data, 1, row_bytes, fp);

        for (int col = 0; col < width; col++) {
            int byte_index = col / 8;
            int bit_index = 7 - (col % 8);
            uint8_t byte = row_data[byte_index];
            output[row][col] = (byte >> bit_index) & 0x01;
        }
    }

    fclose(fp);
    *out_width = width;
    *out_height = height;
    return 0;
}

int main(int argc, char **argv)
{
    uint8_t i2c_node_address = 0;
    int x = -1;
    int y = -1;
    char line[25] = {0};
    char msg[200] = {0};
    char oled_type[10] = {0};
    int clear_line = -1;
    int clear_all = -1;
    int orientation = -1;
    int inverted = -1;
    int display = -1;
    int font = 0;
    
    int cmd_opt = 0;
    
    while(cmd_opt != -1) 
    {
        cmd_opt = getopt(argc, argv, "I:c::d:f:hi:l:m:n:r:x:y:b:");

        /* Lets parse */
        switch (cmd_opt) {
            case 'I':
                snprintf(oled_type, sizeof (oled_type) - 1, "%s", optarg);
                break;
            case 'c':
                if (optarg)
                {
                    clear_line = atoi(optarg);
                }
                else
                {
                    clear_all = 1;
                }
                break;
            case 'd':
                display = atoi(optarg);
                break;
            case 'f':
                font = atoi(optarg);
                break;
            case 'h':
                print_help();
                return 0;
            case 'i':
                inverted = atoi(optarg);
                break;
            case 'l':
                strncpy(line, optarg, sizeof(line));
                break;
            case 'm':
                strncpy(msg, optarg, sizeof(msg));
                break;    
            case 'n':
                i2c_node_address = (uint8_t)atoi(optarg);
                break;
            case 'r':
                orientation = atoi(optarg);
                if (orientation != 0 && orientation != 180)
                {
                    printf("orientation value must be 0 or 180\n");
                    return 1;
                }
                break;
            case 'x':
                x = atoi(optarg);
                break;
            case 'y':
                y = atoi(optarg);
                break;
            case 'b':
                strncpy(bmp_path, optarg, sizeof(bmp_path));
                break;
            case -1:
                // just ignore
                break;
            /* Error handle: Mainly missing arg or illegal option */
            case '?':
                if (optopt == 'I')
                {
                    printf("prams -%c missing oled type (128x64/128x32/64x48)\n", optopt);
                    return 1;
                }
                else if (optopt == 'd' || optopt == 'f' || optopt == 'i')
                {
                    printf("prams -%c missing 0 or 1 fields\n", optopt);
                    return 1;
                }
                else if (optopt == 'l' || optopt == 'm')
                {
                    printf("prams -%c missing string\n", optopt);
                    return 1;
                }
                else if (optopt == 'n')
                {
                    printf("prams -%c missing 0,1,2... I2C device node number\n", optopt);
                    return 1;
                }
                else if (optopt == 'r')
                {
                    printf("prams -%c missing 0 or 180 fields\n", optopt);
                    return 1;
                }
                else if (optopt == 'x' || optopt == 'y')
                {
                    printf("prams -%c missing coordinate values\n", optopt);
                    return 1;
                }
                break;
            default:
                print_help();
                return 1;
        }
    }
    
    uint8_t rc = 0;
    
    // open the I2C device node
    rc = ssd1306_init(i2c_node_address);
    
    if (rc != 0)
    {
        printf("no oled attached to /dev/i2c-%d\n", i2c_node_address);
        return 1;
    }
    
    // init oled module
    if (oled_type[0] != 0)
    {
        if (strcmp(oled_type, "128x64") == 0)
            rc += ssd1306_oled_default_config(64, 128);
        else if (strcmp(oled_type, "128x32") == 0)
            rc += ssd1306_oled_default_config(32, 128);
        else if (strcmp(oled_type, "64x48") == 0)
            rc += ssd1306_oled_default_config(48, 64);
    }
    else if (ssd1306_oled_load_resolution() != 0)
    {
        printf("please do init oled module with correction resolution first!\n");
        return 1;
    }
    
    // clear display
    if (clear_all > -1)
    {
        rc += ssd1306_oled_clear_screen();
    }
    else if (clear_line > -1)
    {
        rc += ssd1306_oled_clear_line(clear_line);
    }
    
    // set rotate orientation
    if (orientation > -1)
    {
        rc += ssd1306_oled_set_rotate(orientation);
    }
    
    // set oled inverted
    if (inverted > -1)
    {
        rc += ssd1306_oled_display_flip(inverted);
    }
    
    // set display on off
    if (display > -1)
    {
        rc += ssd1306_oled_onoff(display);
    }

    // print bitmap image to screen
    if (bmp_path[0]) {
        int w = 0, h = 0;
        if (load_bitmap_to_bool_array(bmp_path, bitmap, &w, &h) == 0) {
            rc += ssd1306_oled_draw_bitmap(bitmap);
        } else {
            fprintf(stderr, "Failed to load or parse bitmap from %s\n", bmp_path);
        }
    }

    // set cursor XY
    if (x > -1 && y > -1)
    {
        rc += ssd1306_oled_set_XY(x, y);
    }
    else if (x > -1)
    {
        rc += ssd1306_oled_set_X(x);
    }
    else if (y > -1)
    {
        rc += ssd1306_oled_set_Y(y);
    }
    
    // print text
    if (msg[0] != 0)
    {
        rc += ssd1306_oled_write_string(font, msg);
    }
    else if (line[0] != 0)
    {
        rc += ssd1306_oled_write_line(font, line);
    }    
    
    // close the I2C device node
    ssd1306_end();
    
    return rc;
}
