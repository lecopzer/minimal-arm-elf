## Introduction
  I write a program byte by byte. 
  This helps us understand how ELF is constructed and the way to relocate outer symbols.

  `main.c`: Basic ELF construction, is only an empty program return 0.
  
  `hello_world.c`: Advanced to relocating symbol such as printf by linker.  

## Prerequest                                                                                                   
	arm-linux-gnueabihf-gcc
	qemu-arm
	arm-linux-gnueabihf library

## Ref
	http://www.muppetlabs.com/~breadbox/software/tiny/teensy.html
	https://github.com/mydzor/tinyelf-arm
