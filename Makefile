all:
	arm-linux-gnueabihf-gcc -O0 main.c
	qemu-arm -L /usr/arm-linux-gnueabihf/ a.out > out
	chmod +x out
	@echo "\n[EXEC]"
	qemu-arm out
	@echo "Success!!"

clean:
	rm -f out a.out
