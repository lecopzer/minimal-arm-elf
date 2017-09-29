#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#define ALIGN 4096

int main() {
  char *buf, *code, *entry, *je, *o;
  const int poolsz = 256 * 1024;
  code = malloc(poolsz);
  buf = malloc(poolsz);
  o = buf = (char*)(((int)buf + ALIGN - 1)  & -ALIGN);
  je = code;
  *(int*)je = 0xe3a07001;       // mov     r7, #1
  je +=4;
  *(int*)je = 0xe3a00000;       // mov     r0, #0
  je +=4;
  *(int*)je = 0xef000000;       // svc 0
  je +=4;

  // elf header
  *o++ = 0x7f; *o++ = 'E'; *o++ = 'L'; *o++ = 'F';
  *o++ = 1;    *o++ = 1;   *o++ = 1;   *o++ = 0;
  o = o + 8;
  *o++ = 2; *o++ = 0; *o++ = 40; *o++ = 0; // e_type 2 = executable & e_machine 40 = ARM
  *(int*)o = 1;           o = o + 4;
  entry = o;              o = o + 4; // e_entry
  *(int*)o = 52;          o = o + 4; // e_phoff
  *(int*)o = 0;           o = o + 4; // e_shoff
  *(int*)o = 0x5000402;   o = o + 4; // e_flags
  *o++ = 52; *o++ = 0;
  *o++ = 32; *o++ = 0; *o++ = 1; *o++ = 0; // e_phentsize & e_phnum
  *o++ =  0; *o++ = 0; *o++ = 0; *o++ = 0; // e_shentsize & e_shnum
  *o++ =  0; *o++ = 0;

  // program header
  *(int*)o = 1;                   o = o + 4;  *(int*)o = 52 + 32;  o = o + 4;
  *(int*)o = (int)o + 24;         o = o + 4;
  *(int*)o = (int)o + 24 - 4;     o = o + 4;
  *(int*)o = (je - code);         o = o + 4;
  *(int*)o = (je - code);         o = o + 4;
  *(int*)o = 5;                   o = o + 4;  *(int*)o = 0x1000;   o = o + 4;

  *(int*)entry = (int)o;
  memcpy(o, code,  je - code);
  o += je - code;

  write(1, buf, o - buf);
  return 0;
}
