PASS_COLOR = \x1b[32;01m
NO_COLOR = \x1b[0m
all:
	@arm-linux-gnueabihf-gcc -g -O0 main.c
	@qemu-arm -L /usr/arm-linux-gnueabihf/ a.out > out
	@chmod +x out
	@/bin/echo -e "\n$(PASS_COLOR)[EXEC for main.c]$(NO_COLOR)"
	@echo "[This program only retruns 0]"
	@qemu-arm -L /usr/arm-linux-gnueabihf/ out
	@/bin/echo -e "$(PASS_COLOR)Pass!!$(NO_COLOR)"
	@arm-linux-gnueabihf-gcc -g -O0 hello_world.c
	@qemu-arm -L /usr/arm-linux-gnueabihf/ a.out > out
	@chmod +x out
	@/bin/echo -e "\n$(PASS_COLOR)[EXEC for hello_world.c]$(NO_COLOR)"
	@echo "[This program prints hello world]"
	@qemu-arm -L /usr/arm-linux-gnueabihf/ out
	@/bin/echo -e "$(PASS_COLOR)Pass!!$(NO_COLOR)"

clean:
	rm -f out a.out
