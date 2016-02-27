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
//    *(int*)je = 0xe3a07001;       // mov     r7, #1
//    *(int*)je = 0xe1a0700d;       // mov     r7, sp
//    je +=4;
    *(int*)je = 0xe3070078;       // mov     r0, #1
    je +=4;
    *(int*)je = 0xe34f0664;       // mov     r0, #1
    je +=4;
    *(int*)je = 0xe3a01000;       // mov     r0, #1
    je +=4;
    *(int*)je = 0xe3a02000;       // mov     r0, #1
    je +=4;
		char* bl_printf_movw = je;
    je +=4;
		char* bl_printf_movt = je;
    je +=4;
    *(int*)je = 0xe28fe000;       // add		lr, pc ,#0
    je +=4;
//    *(int*)je = 0xe59cf000;       // ldr		pc, [ip]
    *(int*)je = 0xe1a0f00c;       // mov		pc, ip
    je +=4;
		char* bl_rand_movw = je;
    je +=4;
		char* bl_rand_movt = je;
    je +=4;
    *(int*)je = 0xe28fe000;       // add		lr, pc ,#0
    je +=4;
//    *(int*)je = 0xe59cf000;       // ldr		pc, [ip]
    *(int*)je = 0xe1a0f00c;       // mov		pc, ip
    je +=4;
    *(int*)je = 0xe3a00000;       // mov     r0, #0
    je +=4;
    *(int*)je = 0xe3a07001;       // mov     r7, #1
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
    *o++ = 32; *o++ = 0; *o++ = 4; *o++ = 0; // e_phentsize & e_phnum
    *o++ = 40; *o++ = 0; *o++ = 14; *o++ = 0; // e_shentsize & e_shnum
    *o++ =  1; *o++ = 0;
		phdr = o;

		char* v_text_record, *s_text;
    to = phdr;
		// program header
		// PT_LOAD for code
    *(int*)to = 1;			to = to + 4;	*(int*)to = 0x1000; to = to + 4;
		v_text_record = to; to = to + 4;
    *(int*)to = (int)to + 123 + 32 +24 + 32 -4;			to = to + 4;
		s_text = to; to = to + 4;
    *(int*)to = (je - code);															to = to + 4;
    *(int*)to = 5;      to = to + 4;	*(int*)to = 0x1000; to = to + 4;

		char* v_data_size;
		// PT_LOAD for data
    *(int*)to = 1;					to = to + 4; *(int*)to = 52 + 32 * 4; to = to + 4;
    *(int*)to = (int)to + 24 + 32*2;																to = to + 4;
    *(int*)to = (int)to + 24 + 32*2 -4;															to = to + 4;
    v_data_size = to;																									to = to + 4;
    *(int*)to = 0x1000;																								to = to + 4;
    *(int*)to = 6;					to = to + 4; *(int*)to = 0x1000;					to = to + 4;

		// PT_INTERP 
    *(int*)to = 3;         to = to + 4; *(int*)to = 52 + 32 * 4; to = to + 4;
    *(int*)to = (int)to + 24 + 32; to = to + 4;
    *(int*)to = (int)to + 24 + 32 - 4;  to = to + 4;
    *(int*)to = 25; to = to + 4;
    *(int*)to = 25; to = to + 4;
    *(int*)to = 4;         to = to + 4; *(int*)to = 1;     to = to + 4;

		char* v_dym;
		char* s_dym;
		int dym_off = 52 + 32 *4 + 25;
    // PT_DYNAMIC
    *(int*)to = 2;           to = to + 4; *(int*)to = dym_off; to = to + 4;
    v_dym = to;													to = to + 4; 
		*(int*)to = (int)to + 24 + 25 -4;		to = to + 4;
    s_dym = to;								to = to + 4; 
		*(int*)to = 80;           to = to + 4;
    *(int*)to = 6;						to = to + 4; *(int*)to = 4;           to = to + 4;

		char* _data = to;
    memcpy(to, "/lib/ld-linux-armhf.so.3", 25);
    to += 25;

		*(int*)v_dym = (int)to;
		*(int*)(v_dym + 4) = (int)to;
		char* v_strtab, *v_symtab, *_v_rel, *_v_got, *__v_rel,*_v_symtab, *_v_plt;
		char* s_strtab, *_s_rel, *__s_rel, *_s_plt,*_s_symtab;
		int s_symtab;
		// .dynamic
    *(int*)to = 5;	to = to + 4; v_strtab = to;   to = to + 4;
    *(int*)to = 10; to = to + 4; s_strtab = to;		to = to + 4;
    *(int*)to = 6;	to = to + 4; _v_symtab = to;		to = to + 4;
    *(int*)to = 11; to = to + 4; _s_symtab = to;		to = to + 4;
    *(int*)to = 1; to = to + 4; *(int*)to = 8; to = to + 4;
    *(int*)to = 1; to = to + 4; *(int*)to = 18; to = to + 4;
    *(int*)to = 17; to = to + 4; _v_rel = to; to = to + 4;
    *(int*)to = 18; to = to + 4; _s_rel = to; to = to + 4;
    *(int*)to = 19; to = to + 4; __s_rel = to; to = to + 4;
    *(int*)to = 3; to = to + 4; _v_got = to; to = to + 4;
    *(int*)to = 2; to = to + 4; *(int*)to = 16; to = to + 4;
    *(int*)to = 20; to = to + 4; *(int*)to = 17; to = to + 4;
    *(int*)to = 23; to = to + 4; __v_rel = to; to = to + 4;
    *(int*)to = 0; to = to + 8;

		// XXX
		// Why we must add 16bytes to let readelf show the end of two entry?	
		*(int*)s_dym = ((int)to - *(int*)v_dym) + 16;
		*(int*)(s_dym + 4) = *(int*)s_dym;

		shstrtab_off = (char*)(to - buf);

		char *shstrtab = "\0.text\0.data\0.bss\0.shstrtab\0.symtab\0.strtab\0"
											".interp\0.dynamic\0.dynsym\0.dynstr\0.rel.plt\0.plt\0"
											".got\0.rodata\0";
		int shstrtab_size = 104;
    memcpy(to, shstrtab, shstrtab_size);
		to += shstrtab_size;

		char *strtab = "\0_start\0printf\0rand\0";
		int strtab_size = 20;
		strtab_off = (char*)(to - buf);
    memcpy(to, strtab, strtab_size);
		to += strtab_size;

		char* v_dynstr = to;
		*(int*)v_strtab = (int)to;
		char* dynstr_off = (char*)(to - buf);
		char *dynstr = "\0printf\0libc.so.6\0libdl.so.2\0rand\0";
		int dynstr_size = 35;
		*(int*)s_strtab = dynstr_size;
    memcpy(to, dynstr, dynstr_size);
		to += dynstr_size;

		char *dynsym_off = (char*)(to - buf);
		char* v_dynsym;
		int s_dynsym;
		v_dynsym = to;
		*(int*)_v_symtab = (int)to;
		memset(to, 0, 16);
		to += 16;
		// .dynsym for printf
    *(int*)to = 1;         to = to + 4; 
		*(int*)to = 0; to = to + 4;
    *(int*)to = 0;         to = to + 4; 
    *to = ELF32_ST_INFO(1, 2); to = to + 1; 
	  *to = 0; to++;
    *to++ = 0;  *to++ = 0;      
		// .dynsym for rand
    *(int*)to = 29;         to = to + 4; 
		*(int*)to = 0; to = to + 4;
    *(int*)to = 0;         to = to + 4; 
    *to = ELF32_ST_INFO(1, 2); to = to + 1; 
	  *to = 0; to++;
    *to++ = 0;  *to++ = 0;      

		s_dynsym = (int)to - (int)v_dynsym;
		*(int*)_s_symtab = (int)to - (int)v_dynsym;

		symtab_off = (char*)(to - buf);
		v_symtab = to;
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
		// symtab for .interp
    *(int*)to = 0;         to = to + 4; 
		*(int*)to = (int)_data; to = to + 4;
    *(int*)to = 0;         to = to + 4; 
    *to = ELF32_ST_INFO(0, 3); to = to + 1; 
	  *to = 0; to++;
    *to++ = 6;  *to++ = 0;      
		// symtab for .dynamic
    *(int*)to = 0;         to = to + 4; 
		*(int*)to = *(int*)v_dym; to = to + 4;
    *(int*)to = 0;         to = to + 4; 
    *to = ELF32_ST_INFO(0, 3); to = to + 1; 
	  *to = 0; to++;
    *to++ = 7;  *to++ = 0;      
		// symtab for .dymsym
    *(int*)to = 0;         to = to + 4; 
		*(int*)to = (int)v_dynsym; to = to + 4;
    *(int*)to = 0;         to = to + 4; 
    *to = ELF32_ST_INFO(0, 3); to = to + 1; 
	  *to = 0; to++;
    *to++ = 8;  *to++ = 0;      
		// symtab for .dymstr
    *(int*)to = 0;         to = to + 4; 
		*(int*)to = (int)v_dynstr; to = to + 4;
    *(int*)to = 0;         to = to + 4; 
    *to = ELF32_ST_INFO(0, 3); to = to + 1; 
	  *to = 0; to++;
    *to++ = 9;  *to++ = 0;      
		// symtab for .rel.plt
    *(int*)to = 0;         to = to + 4; 
		char* symtab_v_rel = to; to = to + 4;
    *(int*)to = 0;         to = to + 4; 
    *to = ELF32_ST_INFO(0, 3); to = to + 1; 
	  *to = 0; to++;
    *to++ = 10;  *to++ = 0;      
		// symtab for .plt
    *(int*)to = 0;         to = to + 4; 
		char* symtab_v_plt = to; to = to + 4;
    *(int*)to = 0;         to = to + 4; 
    *to = ELF32_ST_INFO(0, 3); to = to + 1; 
	  *to = 0; to++;
    *to++ = 11;  *to++ = 0;      
		// symtab for .got
    *(int*)to = 0;         to = to + 4; 
		char* symtab_v_got = to; to = to + 4;
    *(int*)to = 0;         to = to + 4; 
    *to = ELF32_ST_INFO(0, 3); to = to + 1; 
	  *to = 0; to++;
    *to++ = 12;  *to++ = 0;      
		char* v_start;
		// symtab for _start
    *(int*)to = 1;         to = to + 4; 
		v_start = to; to = to + 4;
    *(int*)to = 0;         to = to + 4; 
    *to = ELF32_ST_INFO(1, 2); to = to + 1; 
	  *to = 0; to++;
    *to++ = 2;  *to++ = 0;      
		// .dynsym for printf
    *(int*)to = 8;         to = to + 4; 
		*(int*)to = 0; to = to + 4;
    *(int*)to = 0;         to = to + 4; 
    *to = ELF32_ST_INFO(1, 2); to = to + 1; 
	  *to = 0; to++;
    *to++ = 0;  *to++ = 0;      
		// .dynsym for printf
    *(int*)to = 15;         to = to + 4; 
		*(int*)to = 0; to = to + 4;
    *(int*)to = 0;         to = to + 4; 
    *to = ELF32_ST_INFO(1, 2); to = to + 1; 
	  *to = 0; to++;
    *to++ = 0;  *to++ = 0;      


		s_symtab = (int)to - (int)v_symtab;


//-------------------------.got
		char* v_got = to;
		*(int*) _v_got = (int)to;
		*(int*) symtab_v_got = (int)to;
		char* got_off = (char*)(to - buf);

    *(int*)to = *(int*)v_dym;         to = to + 4; 
		to += 4;	// reserved 2 and 3 entry for linker
		/*
		*(int*)to_got_movw = 0xe300a000 | (0xfff & (int)(to)) | (0xf0000 & ((int)(to) << 4));
		*(int*)to_got_movt = 0xe340a000 | (0xfff & ((int)(to)>>16)) | (0xf0000 & ((int)(to) >> 12));
		*/
    char* to_got_movw = to;         
    char* to_got_movt = to;         
		to += 4;	// reserved 2 and 3 entry for linker

		char* got_printf = to;

		char * v_plt1 = to; to += 4;
//    *(int*)to = (int)v_plt;         to = to + 4; //	.plt entry

		char* _got_rand = to;

		char * v_plt2 = to; to+=4;
//    *(int*)to = (int)v_plt;         to = to + 4; //	.plt entry
    to = to + 4; // end with 0x0
		int got_size = (int)to - (int)v_got;

//-------------------------------------------
    to =    (char*)(((int)to + (4095)) & -4096);

		char *_text = to;
		*(int*) v_text_record = (int)to;
		*(int*) entry = (int)to;
		*(int*) v_start = (int)to;
		*(int*) (v_text_record+4) = (int)to;
		*(int*) v_data_size = (int)_text - (int)_data;
		*(int*) (v_data_size+4) = (int)_text - (int)_data;

		

		char *code_r = to;
//    memcpy(to, code,  je - code);
		text_off = (char*)(to - buf);
		to += je - code;

		
//------------------------------.plt
		char* v_plt = to;
		*(int*) v_plt1 = (int)to;
		*(int*) v_plt2 = (int)to;
		*(int*) symtab_v_plt = (int)to;
//		*(int*) _v_plt = (int)to;
		char* plt_off = (char*)(to - buf);

		/*
		*(int*)to_rand_movw = 0xe300c000 | (0xfff & (int)(to)) | (0xf0000 & ((int)(to) << 4));
		*(int*)to_rand_movt = 0xe340c000 | (0xfff & ((int)(to)>>16)) | (0xf0000 & ((int)(to) >> 12));
		*(int*)to_printf_movw = 0xe300c000 | (0xfff & (int)(to)) | (0xf0000 & ((int)(to) << 4));
		*(int*)to_printf_movt = 0xe340c000 | (0xfff & ((int)(to)>>16)) | (0xf0000 & ((int)(to) >> 12));
		*/
    *(int*)to = 0xe52de004;         to = to + 4; // push {lr}
		*(int*)to = 0xe300a000 | (0xfff & (int)(to_got_movw)) | (0xf0000 & ((int)(to_got_movw) << 4));
		to = to + 4;
		*(int*)to = 0xe340a000 | (0xfff & ((int)(to_got_movt)>>16)) | (0xf0000 & ((int)(to_got_movt) >> 12));
		to = to + 4;
//    char* to_got_movw = to;         to = to + 4; // bl .got
//    char* to_got_movt = to;         to = to + 4; // bl .got
    *(int*)to = 0xe1a0e00a;					to = to + 4;// mov lr,r10
    *(int*)to = 0xe59ef000;					to = to + 4;// ldr pc, [lr]

		// printf
		*(int*)bl_printf_movw = 0xe300c000 | (0xfff & (int)(to)) | (0xf0000 & ((int)(to) << 4));
		*(int*)bl_printf_movt = 0xe340c000 | (0xfff & ((int)(to)>>16)) | (0xf0000 & ((int)(to) >> 12));

		*(int*) to = 0xe300c000 | (0xfff & (int)(got_printf)) | (0xf0000 & ((int)(got_printf) << 4));
		to += 4;
		*(int*) to = 0xe340c000 | (0xfff & ((int)(got_printf)>>16)) | (0xf0000 & ((int)(got_printf) >> 12));
		to += 4;
 //   *(int*)to = to_plt_movw;					to = to + 4;// ldr		pc, [ip]
 //   *(int*)to = to_plt_movt;					to = to + 4;// ldr		pc, [ip]
		/*
    char* to_printf_movw = to;         to = to + 4; // bl printf
    char* to_printf_movt = to;         to = to + 4; // bl printf
		*/
    *(int*)to = 0xe59cf000;					to = to + 4;// ldr		pc, [ip]
//    *(int*)to = 0xe1a0f00c;					to = to + 4;// mov		pc, ip
		
		// rand
		*(int*)bl_rand_movw = 0xe300c000 | (0xfff & (int)(to)) | (0xf0000 & ((int)(to) << 4));
		*(int*)bl_rand_movt = 0xe340c000 | (0xfff & ((int)(to)>>16)) | (0xf0000 & ((int)(to) >> 12));
		/*
    char* to_rand_movw = to;         to = to + 4; // bl printf
    char* to_rand_movt = to;         to = to + 4; // bl printf
		*/
		*(int*) to = 0xe300c000 | (0xfff & (int)(_got_rand)) | (0xf0000 & ((int)(_got_rand) << 4));
		to += 4;
		*(int*) to = 0xe340c000 | (0xfff & ((int)(_got_rand)>>16)) | (0xf0000 & ((int)(_got_rand) >> 12));
		to += 4;
//    *(int*)to = to_plt_movw;					to = to + 4;// ldr		pc, [ip]
//    *(int*)to = to_plt_movt;					to = to + 4;// ldr		pc, [ip]
    *(int*)to = 0xe59cf000;					to = to + 4;// ldr		pc, [ip]
//    *(int*)to = 0xe1a0f00c;					to = to + 4;// mov		pc, ip

		int plt_size = (int)to - (int)v_plt;
//		*(int*) _s_plt = plt_size;

//---------------------------------------
		char* v_rel = to;
		*(int*) symtab_v_rel = (int)to;
		*(int*) _v_rel = (int)to;
		*(int*) __v_rel = (int)to;
		char* rel_off = (char*)(to - buf);
    *(int*)to = (int)got_printf;         to = to + 4; 
    *(int*)to = 0x16 | 1 << 8 ;					 to = to + 4; // 0x16 R_ARM_JUMP_SLOT | .dymsyn index << 8
    *(int*)to = (int)_got_rand;         to = to + 4; 
    *(int*)to = 0x16 | 2 << 8 ;					 to = to + 4; // 0x16 R_ARM_JUMP_SLOT | .dymsyn index << 8
		
		int rel_size = (int)to - (int)v_rel;
		*(int*) _s_rel = rel_size;
		*(int*) __s_rel = 8;


		char* _rodata = "Hello world!!\n\0";
		char*  v_rodata = to;
		char* rodata_off = (char*)(to - buf);
		int rodata_size = 15;
    memcpy(to, _rodata, rodata_size);
		to += rodata_size;

		*(int*) s_text = (int)to - (int)_text;
		*(int*) (s_text+4) = (int)to - (int)_text;

    memcpy(code_r, code,  je - code);


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
    *(int*)to = s_symtab;						to = to + 4;
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

		// .interp	6
    *(int*)to = 44;         to = to + 4; *(int*)to = 1; to = to + 4;
    *(int*)to = 2; to = to + 4;
    *(int*)to = (int)_data;  to = to + 4;
    *(int*)to = 52 + 32 * 3; to = to + 4;
    *(int*)to = 25; to = to + 4;
    *(int*)to = 0;         to = to + 4; *(int*)to = 0;     to = to + 4;
    *(int*)to = 1;         to = to + 4; *(int*)to = 0;     to = to + 4;

		// .dynamic	7
    *(int*)to = 52;         to = to + 4; *(int*)to = 6; to = to + 4;
    *(int*)to = 2; to = to + 4;
    *(int*)to = *(int*)v_dym;  to = to + 4;
    *(int*)to = dym_off; to = to + 4;
    *(int*)to = *(int*)s_dym; to = to + 4;
    *(int*)to = 9;         to = to + 4; *(int*)to = 0;     to = to + 4;
    *(int*)to = 4;         to = to + 4; *(int*)to = 0;     to = to + 4;

		// .dynsym		8
    *(int*)to = 61;         to = to + 4; *(int*)to = 11;			to = to + 4;
    *(int*)to = 2; to = to + 4;
    *(int*)to = (int)v_dynsym; to = to + 4;
    *(int*)to = (int)dynsym_off;		to = to + 4;
    *(int*)to = s_dynsym;						to = to + 4;
    *(int*)to = 9;         to = to + 4; *(int*)to = 123;     to = to + 4;
    *(int*)to = 4;         to = to + 4; *(int*)to = 16;     to = to + 4;

		// .dynstr	9
    *(int*)to = 69;         to = to + 4; *(int*)to = 3;			to = to + 4;
    *(int*)to = 2; to = to + 4;
    *(int*)to = (int)v_dynstr; to = to + 4;
    *(int*)to = (int)dynstr_off;		to = to + 4;
    *(int*)to = dynstr_size;				to = to + 4;
    *(int*)to = 0;         to = to + 4; *(int*)to = 0;			to = to + 4;
    *(int*)to = 1;         to = to + 4; *(int*)to = 0;			to = to + 4;

		// .rel.plt	10
    *(int*)to = 77;         to = to + 4; *(int*)to = 9;			to = to + 4;
    *(int*)to = 2 | 0x40 ; to = to + 4;
    *(int*)to = (int)v_rel; to = to + 4;
    *(int*)to = (int)rel_off;		to = to + 4;
    *(int*)to = rel_size;				to = to + 4;
    *(int*)to = 8;         to = to + 4; *(int*)to = 11;			to = to + 4;
    *(int*)to = 4;         to = to + 4; *(int*)to = 0x8;			to = to + 4;

		// .plt	11
    *(int*)to = 86;         to = to + 4; *(int*)to = 1;			to = to + 4;
    *(int*)to = 6; to = to + 4;
    *(int*)to = (int)v_plt; to = to + 4;
    *(int*)to = (int)plt_off;		to = to + 4;
    *(int*)to = plt_size;				to = to + 4;
    *(int*)to = 0;         to = to + 4; *(int*)to = 0;			to = to + 4;
    *(int*)to = 4;         to = to + 4; *(int*)to = 4;			to = to + 4;

		// .got	12
    *(int*)to = 91;         to = to + 4; *(int*)to = 1;			to = to + 4;
    *(int*)to = 3; to = to + 4;
    *(int*)to = (int)v_got; to = to + 4;
    *(int*)to = (int)got_off;		to = to + 4;
    *(int*)to = got_size;				to = to + 4;
    *(int*)to = 0;         to = to + 4; *(int*)to = 0;			to = to + 4;
    *(int*)to = 4;         to = to + 4; *(int*)to = 4;			to = to + 4;

		// .rodata	13
    *(int*)to = 96;         to = to + 4; *(int*)to = 1;			to = to + 4;
    *(int*)to = 2; to = to + 4;
    *(int*)to = (int)v_rodata; to = to + 4;
    *(int*)to = (int)rodata_off;		to = to + 4;
    *(int*)to = rodata_size;				to = to + 4;
    *(int*)to = 0;         to = to + 4; *(int*)to = 0;			to = to + 4;
    *(int*)to = 4;         to = to + 4; *(int*)to = 0;			to = to + 4;

    write(1, buf, to - buf);

return 0;
}
