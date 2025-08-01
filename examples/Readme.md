## Examples

### FMV Player
Plays a full motion video (bad apple is included for testing) on the OLED, uses an animated bitmap file.

Bitmap file was generated using
```
$ mkdir frames && cd frames
$ ffmpeg -i /path/to/input.mp4 -vf "scale=128:64,format=gray" -pix_fmt monow -vsync 0 frame_%04d.bmp
$ cat frame_* >> ../output.abmp
```

Usage:
```
$ sudo ./ssd1306_fmv [-I 128x64] -n <device> -r <0/128> -a <animation file> -d <frame delay in ms>
```

Example:
```
$ sudo ./ssd1306_fmv -I 128x64 -n 1 -r 0 -a bad_apple.abmp -d 33
```
