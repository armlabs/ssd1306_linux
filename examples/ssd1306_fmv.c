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
#include <sys/time.h>

#include "ssd1306.h"

void print_help()
{
    printf("help message\n\n");
    printf("ssd1306_fmv [-I 128x64] -n <device> -r <0/128> -a <animation file> -d <frame delay in ms>\n");
    printf("-I\t\tinit oled (128x32 or 128x64 or 64x48)\n");
    printf("-h\t\thelp message\n");
    printf("-n\t\tI2C device node address (0,1,2..., default 0)\n");
    printf("-r\t\t0/normal 180/rotate\n");
    printf("-a\t\tanimation file\n");
    printf("-d\t\tframe delay in ms\n");
}

#define MAX_WIDTH 128
#define MAX_HEIGHT 64
bool bitmap[MAX_HEIGHT][MAX_WIDTH];
char anim_path[256] = {0};

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
    uint8_t i2c_node_address = 1;
    char oled_type[10] = {0};
    int orientation = -1;
    int y = 0;
    int cmd_opt = 0;
    
    while(cmd_opt != -1) 
    {
        cmd_opt = getopt(argc, argv, "I:n:r:a:d:h:");

        /* Lets parse */
        switch (cmd_opt) {
            case 'I':
                snprintf(oled_type, sizeof (oled_type) - 1, "%s", optarg);
                break;
            case 'h':
                print_help();
                return 0;
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
            case 'a':
                strncpy(anim_path, optarg, sizeof(anim_path));
                break;
            case 'd':
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
                else if (optopt == 'd')
                {
                    printf("prams -%c missing delay values\n", optopt);
                    return 1;
                }
                else if (optopt == 'a')
                {
                    printf("prams -%c missing file name\n", optopt);
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
    
    rc += ssd1306_oled_clear_screen();
    
    // set rotate orientation
    if (orientation > -1)
    {
        rc += ssd1306_oled_set_rotate(orientation);
    }
    
    // display animation
if (anim_path[0]) {
        FILE *fp = fopen(anim_path, "rb");
        if (!fp) {
            perror("fopen anim_path");
            return 1;
        }

        const int BMP_FRAME_SIZE = 1086; // 128x64 1bpp BMP
        uint8_t frame_buf[BMP_FRAME_SIZE];
        int frame_num = 0;

        while (fread(frame_buf, 1, BMP_FRAME_SIZE, fp) == BMP_FRAME_SIZE) {
            struct timeval start, end;
            gettimeofday(&start, NULL);

            // Save to temp file
            char temp_name[] = "/tmp/frameXXXXXX.bmp";
            int temp_fd = mkstemps(temp_name, 4);
            if (temp_fd < 0) {
                perror("mkstemps");
                fclose(fp);
                return 1;
            }

            if (write(temp_fd, frame_buf, BMP_FRAME_SIZE) != BMP_FRAME_SIZE) {
                perror("write temp bmp");
                close(temp_fd);
                unlink(temp_name);
                break;
            }
            close(temp_fd);

            int w = 0, h = 0;
            if (load_bitmap_to_bool_array(temp_name, bitmap, &w, &h) == 0) {
                rc += ssd1306_oled_draw_bitmap(bitmap);
            } else {
                fprintf(stderr, "Failed to parse BMP frame %d\n", frame_num);
            }

            unlink(temp_name);

            gettimeofday(&end, NULL);

            // Compute elapsed time in ms
            long elapsed_ms = (end.tv_sec - start.tv_sec) * 1000L +
                              (end.tv_usec - start.tv_usec) / 1000L;

            long delay_ms = y - elapsed_ms;
            if (delay_ms > 0) {
                usleep(delay_ms * 1000);
            }

            frame_num++;
        }

        fclose(fp);
    }
    
    // close the I2C device node
    ssd1306_end();
    
    return rc;
}
