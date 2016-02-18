#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#define ALIGN 4096
#define ELF32_ST_INFO(b,t) (((b)<<4)+((t)&0xf))

int main() {

    char *o, *buf, *code, *entry, *je, *tje, *to, *phdr;
		char *shstrtab_off, *strtab_off,*data_off, *text_off, *symtab_off;
		const int poolsz = 256 * 1024;
    code = malloc(poolsz);
    buf = malloc(poolsz);
    o = buf = (char*)(((int)buf + ALIGN - 1)  & -ALIGN);
    je = code;
    *(int*)je = 0xe3a07001;       // mov     r7, #1
    je +=4;
    *(int*)je = 0xe3a0002a;       // mov     r0, #42
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
    *(int*)o = 0x2000;           o = o + 4; // e_shoff
    *(int*)o = 0x5000402;   o = o + 4; // e_flags
    *o++ = 52; *o++ = 0;
    *o++ = 32; *o++ = 0; *o++ = 2; *o++ = 0; // e_phentsize & e_phnum
    *o++ = 40; *o++ = 0; *o++ = 6; *o++ = 0; // e_shentsize & e_shnum
    *o++ =  1; *o++ = 0;
		phdr = o;

		char* v_text_record;
    to = phdr;
		// program header
		// PT_LOAD for code
    *(int*)to = 1;			to = to + 4;	*(int*)to = 0x1000; to = to + 4;
		v_text_record = to; to = to + 4;
    *(int*)to = (int)to + 123 + 32 +24 + 32 -4  -32 ;			to = to + 4;
    *(int*)to = (je - code);															to = to + 4;
    *(int*)to = (je - code);															to = to + 4;
    *(int*)to = 5;      to = to + 4;	*(int*)to = 0x1000; to = to + 4;

		char* v_data_size;
		// PT_LOAD for data
    *(int*)to = 1;					to = to + 4; *(int*)to = 52 + 32 * 3 -32; to = to + 4;
    *(int*)to = (int)to + 24 + 32 -32;																to = to + 4;
    *(int*)to = (int)to + 24 + 32 -4 -32;															to = to + 4;
    v_data_size = to;																									to = to + 4;
    *(int*)to = 0x1000;																								to = to + 4;
    *(int*)to = 6;					to = to + 4; *(int*)to = 0x1000;					to = to + 4;

		char* _data = to;
		shstrtab_off = (char*)(to - buf);

		char *shstrtab = "\0.text\0.data\0.bss\0.shstrtab\0.symtab\0.strtab\0"
											".interp\0";
		int shstrtab_size = 52;
    memcpy(to, shstrtab, shstrtab_size);
		to += shstrtab_size;

		char *strtab = "\0_start\0";
		int strtab_size = 8;
		strtab_off = (char*)(to - buf);
    memcpy(to, strtab, strtab_size);
		to += strtab_size;

		symtab_off = (char*)(to - buf);
		// first symtab must 0
		memset(to, 0, 16);
		to += 16;
		// symtab for .text
    *(int*)to = 0;         to = to + 4; 
		*(int*)to = (int)to + 32 + 12; to = to + 4;
    *(int*)to = 0;         to = to + 4; 
    *to = ELF32_ST_INFO(0, 3); to = to + 1; 
	  *to = 0; to++;
    *to++ = 2;  *to++ = 0;      
		// symtab for .data
    *(int*)to = 0;         to = to + 4; 
		*(int*)to = (int)_data; to = to + 4;
    *(int*)to = 0;         to = to + 4; 
    *to = ELF32_ST_INFO(0, 3); to = to + 1; 
	  *to = 0; to++;
    *to++ = 3;  *to++ = 0;      
		// symtab for _start
    *(int*)to = 1;         to = to + 4; 
		*(int*)to = (int)to + 12; to = to + 4;
    *(int*)to = 0;         to = to + 4; 
    *to = ELF32_ST_INFO(1, 2); to = to + 1; 
	  *to = 0; to++;
    *to++ = 2;  *to++ = 0;      

    to =    (char*)(((int)to + (4095)) & -4096);

		char *_text = to;
		*(int*) v_text_record = (int)to;
		*(int*) entry = (int)to;
		*(int*) (v_text_record+4) = (int)to;
		*(int*) v_data_size = (int)_text - (int)_data;
		*(int*) (v_data_size+4) = (int)_text - (int)_data;
    memcpy(to, code,  je - code);
		text_off = (char*)(to - buf);
		to += je - code;
		data_off = (char*)(to - buf);
		to += 8;
    to =    (char*)(((int)to + (4095)) & -4096);

		// shdr
		// index 0
    *(int*)to = 0;         to = to + 4; *(int*)to = 0;		 to = to + 4;
    *(int*)to = 0; to = to + 4;
    *(int*)to = 0; to = to + 4;
    *(int*)to = 0; to = to + 4;
    *(int*)to = 0; to = to + 4;
    *(int*)to = 0;         to = to + 4; *(int*)to = 0;     to = to + 4;
    *(int*)to = 0;         to = to + 4; *(int*)to = 0;     to = to + 4;

		// .shstrtab 1
    *(int*)to = 18;         to = to + 4; *(int*)to = 3;		 to = to + 4;
    *(int*)to = 0; to = to + 4;
    *(int*)to = 0; to = to + 4;
    *(int*)to = (int)shstrtab_off;	to = to + 4;
    *(int*)to = shstrtab_size;			to = to + 4;
    *(int*)to = 0;         to = to + 4; *(int*)to = 0;     to = to + 4;
    *(int*)to = 1;         to = to + 4; *(int*)to = 0;     to = to + 4;

		// .text		2
    *(int*)to = 1;					 to = to + 4; *(int*)to = 1;	 to = to + 4;
    *(int*)to = 6; to = to + 4;
    *(int*)to = (int)_text;					to = to + 4;
    *(int*)to = (int)text_off;			to = to + 4;
    *(int*)to = je - code; to = to + 4;
    *(int*)to = 0;         to = to + 4; *(int*)to = 0;     to = to + 4;
    *(int*)to = 4;         to = to + 4; *(int*)to = 0;     to = to + 4;

		// .data		3
    *(int*)to = 7;         to = to + 4; *(int*)to = 1; to = to + 4;
    *(int*)to = 3; to = to + 4;
    *(int*)to = (int)_data ;				to = to + 4;
    *(int*)to = 52 + 32 * 2;				to = to + 4;
    *(int*)to = *(int*)v_data_size; to = to + 4;
    *(int*)to = 0;         to = to + 4; *(int*)to = 0;     to = to + 4;
    *(int*)to = 4;         to = to + 4; *(int*)to = 0;     to = to + 4;

		// .symtab		4
    *(int*)to = 28;         to = to + 4; *(int*)to = 2;			to = to + 4;
    *(int*)to = 0; to = to + 4;
    *(int*)to = 0; to = to + 4;
    *(int*)to = (int)symtab_off;		to = to + 4;
    *(int*)to = 16 << 2;						to = to + 4;
    *(int*)to = 5;         to = to + 4; *(int*)to = 93;     to = to + 4;
    *(int*)to = 4;         to = to + 4; *(int*)to = 16;     to = to + 4;

		// .strtab	5
    *(int*)to = 36;         to = to + 4; *(int*)to = 3;			to = to + 4;
    *(int*)to = 0; to = to + 4;
    *(int*)to = 0; to = to + 4;
    *(int*)to = (int)strtab_off;		to = to + 4;
    *(int*)to = strtab_size;				to = to + 4;
    *(int*)to = 0;         to = to + 4; *(int*)to = 0;			to = to + 4;
    *(int*)to = 1;         to = to + 4; *(int*)to = 0;			to = to + 4;

    write(1, buf, to - buf);

return 0;
}
