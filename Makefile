CC=gcc
CFLAGS=-I ./
LDFLAGS=-static
OBJS=main.o ssd1306.o linux_i2c.o
BIN=ssd1306_bin

%.o:%.c
		$(CC) -c -o $@ $< $(CFLAGS)

$(BIN):$(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

clean:
	rm -f ./*.o $(BIN)

