CC=gcc
CFLAGS=-I ./ -Wall -Werror
LDFLAGS=-static
OBJS=main.o ssd1306.o linux_i2c.o
BIN=ssd1306_bin

default: $(BIN)
.PHONY: default clean

# Adapted from scottmcpeak.com/autodepend/autodepend.html
-include $(OBJS:.o=.d)
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $*.o
	$(CC) -MM $(CFLAGS) $< > $*.d
	@cp -f $*.d $*.d.tmp
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

$(BIN):$(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

clean:
	rm -f *.o *.d $(BIN)

