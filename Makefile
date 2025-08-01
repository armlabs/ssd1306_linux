CC = gcc
CFLAGS = -I ./ -Wall -Werror -fPIC
LDFLAGS = -L. -lssd1306
SO_FLAGS = -shared
BIN = ssd1306_cli
LIB = libssd1306.so

SRC_LIB = ssd1306.c linux_i2c.c
OBJ_LIB = $(SRC_LIB:.c=.o)

SRC_BIN = main.c
OBJ_BIN = $(SRC_BIN:.c=.o)

default: $(BIN)

.PHONY: default clean install

# Autodependency generation
-include $(OBJ_LIB:.o=.d)
-include $(OBJ_BIN:.o=.d)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $*.o
	$(CC) -MM $(CFLAGS) $< > $*.d
	@cp -f $*.d $*.d.tmp
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

# Build the shared library
$(LIB): $(OBJ_LIB)
	$(CC) $(SO_FLAGS) -o $@ $(OBJ_LIB)

# Build the CLI tool, linking to the shared lib
$(BIN): $(OBJ_BIN) $(LIB)
	$(CC) $(CFLAGS) -o $@ $(OBJ_BIN) $(LDFLAGS)

# Install CLI and shared library
install: $(BIN) $(LIB)
	install -Dm755 $(BIN) /usr/local/bin/$(BIN)
	install -Dm755 $(LIB) /usr/local/lib/$(LIB)
	install -Dm644 ssd1306.h /usr/local/include/ssd1306.h
	install -Dm644 linux_i2c.h /usr/local/include/linux_i2c.h
	ldconfig

clean:
	rm -f *.o *.d $(BIN) $(LIB)
