#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>

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
    printf("-l\t\tput your line to display\n");
    printf("-m\t\tput your strings to oled\n");
    printf("-n\t\tI2C device node address (0,1,2..., default 0)\n");
    printf("-r\t\t0/normal 180/rotate\n");
    printf("-x\t\tx position\n");
    printf("-y\t\ty position\n");
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
        cmd_opt = getopt(argc, argv, "I:c::d:f:hi:l:m:n:r:x:y:");

        /* Lets parse */
        switch (cmd_opt) {
            case 'I':
                strncpy(oled_type, optarg, sizeof(oled_type));
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
