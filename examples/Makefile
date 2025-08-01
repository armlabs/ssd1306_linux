CC = gcc
CFLAGS = -Wall -O2
LDFLAGS = -lssd1306
TARGET = ssd1306_fmv
SRC = ssd1306_fmv.c

all: fmv

fmv: $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)
