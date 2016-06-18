PASS_COLOR = \x1b[32;01m
NO_COLOR = \x1b[0m
MAIN_OUT=main_out
HELLO_OUT=hello_world_out
TARGET=main hello_world
CC=arm-linux-gnueabihf-gcc
CFLAGS=-g -O0
EXEC=qemu-arm -L /usr/arm-linux-gnueabihf/

all: $(TARGET)

main:
	@$(CC) $(CFLAGS)  $@.c -o $@
	@$(EXEC) $@ > $(MAIN_OUT)
	@chmod +x $(MAIN_OUT)
	@/bin/echo -e "\n$(PASS_COLOR)[EXEC for $@.c]$(NO_COLOR)"
	@echo "[This program only retruns 0]"
	@$(EXEC) $(MAIN_OUT)
	@/bin/echo -e "$(PASS_COLOR)Pass!!$(NO_COLOR)"

hello_world:
	@$(CC) $(CFLAGS) $@.c -o $@
	@$(EXEC) $@ > $(HELLO_OUT)
	@chmod +x $(HELLO_OUT)
	@/bin/echo -e "\n$(PASS_COLOR)[EXEC for $@.c]$(NO_COLOR)"
	@echo "[This program prints hello world]"
	@$(EXEC) $(HELLO_OUT)
	@/bin/echo -e "$(PASS_COLOR)Pass!!$(NO_COLOR)"

.PHONY: main hello_world

clean:
	rm -f $(MAIN_OUT) $(HELLO_OUT) $(TARGET)
