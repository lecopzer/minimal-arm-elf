#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#define ALIGN 4096
#define ELF32_ST_INFO(b,t) (((b)<<4)+((t)&0xf))

int main() {

    char *o, *buf, *code, *entry, *je, *tje, *to, *phdr;
		const int poolsz = 256 * 1024;
    code = malloc(poolsz);
    buf = malloc(poolsz);
    o = buf = (char*)(((int)buf + ALIGN - 1)  & -ALIGN);
    je = code;
    *(int*)je = 0xe3a07001;       // mov     r7, #1
    je +=4;
    *(int*)je = 0xe3a00000;       // mov     r0, #42
    je +=4;
    *(int*)je = 0xef000000;       // svc 0
    je +=4;

		// elf header
    *o++ = 0x7f; *o++ = 'E'; *o++ = 'L'; *o++ = 'F';
    *o++ = 1;    *o++ = 1;   *o++ = 1;   *o++ = 0;
    o = o + 8;
    *o++ = 2; *o++ = 0; *o++ = 40; *o++ = 0; // e_type 2 = executable & e_machine 40 = ARM
    *(int*)o = 1;           o = o + 4;
    entry = o;   o = o + 4; // e_entry
    *(int*)o = 52;          o = o + 4; // e_phoff
    *(int*)o = 0;           o = o + 4; // e_shoff
    *(int*)o = 0x5000402;   o = o + 4; // e_flags
    *o++ = 52; *o++ = 0;
    *o++ = 32; *o++ = 0; *o++ = 1; *o++ = 0; // e_phentsize & e_phnum
    *o++ =  0; *o++ = 0; *o++ = 0; *o++ = 0; // e_shentsize & e_shnum
    *o++ =  0; *o++ = 0;
		phdr = o;

    to = phdr;
		// program header
		// PT_LOAD for code
    *(int*)to = 1;			to = to + 4;	*(int*)to = 52 + 32; to = to + 4;
    *(int*)to = (int)to + 24 ;			to = to + 4;
    *(int*)to = (int)to + 24 - 4;			to = to + 4;
    *(int*)to = (je - code);															to = to + 4;
    *(int*)to = (je - code);															to = to + 4;
    *(int*)to = 5;      to = to + 4;	*(int*)to = 0x1000; to = to + 4;

		*(int*) entry = (int)to;
    memcpy(to, code,  je - code);
		to += je - code;

    write(1, buf, to - buf);

return 0;
}
