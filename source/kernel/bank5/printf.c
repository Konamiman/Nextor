/*
   Simplified printf and sprintf for SDCC+Z80
   (c) 2018 Konamiman - www.konamiman.com

   This version is about 1.5K smaller than the one contained
   in the z80 library supplied with SDCC

   To compile:
   sdcc -mz80 -c --disable-warning 85 --disable-warning 196 --max-allocs-per-node 100000 --allow-unsafe-read --opt-code-size printf.c

   Supported format specifiers:

   %d or %i: signed int
   %ud or %ui: unsigned int
   %x: hexadecimal int
   %c: character
   %s: string
   %%: a % character

   Also if SUPPORT_LONG is defined:

   %l: signed long
   %ul: unsigned long
   %lx: hexadecimal long
*/

//#define SUPPORT_LONG

#include <stdarg.h>
#include "system.h"

#ifdef SUPPORT_LONG
extern void _ultoa(long val, char* buffer, char base);
extern void _ltoa(long val, char* buffer, char base);
#endif
extern void _uitoa(int val, char* buffer, char base);
extern void _itoa(int val, char* buffer, char base);

static int format_string(const char* buf, const char *fmt, va_list ap);

int printf(const char *fmt, ...)
{
  va_list arg;
  va_start(arg, fmt);
  return format_string(0, fmt, arg);
}

int sprintf(const char* buf, const char* fmt, ...)
{
  va_list arg;
  va_start(arg, fmt);
  return format_string(buf, fmt, arg);
}

static void do_char(const char* buf, char c) __naked
{
  __asm

  ld hl,#4
  add hl,sp
  ld e,(hl)

  dec hl
  ld a,(hl)
  dec hl
  ld l,(hl)
  ld h,a
  or l

  ld a,e
  jp z,CHPUT

  ld(hl),e
  ret

  __endasm;
}

#define do_char_inc(c) {do_char(bufPnt,c); if(bufPnt) { bufPnt++; } count++;}

static int format_string(const char* buf, const char *fmt, va_list ap)
{
  char *fmtPnt;
  char *bufPnt;
  char base;
#ifdef SUPPORT_LONG
  char isLong;
#endif
  char isUnsigned;
  char *strPnt;
  long val;
  static char buffer[16];
  char theChar;
  int count=0;

  fmtPnt = fmt;
  bufPnt = buf;

  while((theChar = *fmtPnt)!=0)
  {
  #ifdef SUPPORT_LONG
    isLong = 0;
  #endif
    isUnsigned = 0;
    base = 10;

    fmtPnt++;

    if(theChar != '%') {
      do_char_inc(theChar);
      continue;
    }

    theChar = *fmtPnt;
    fmtPnt++;

    if(theChar == 's')
    {
      strPnt = va_arg(ap, char *);
      while((theChar = *strPnt++) != 0) 
        do_char_inc(theChar);

      continue;
    } 

    if(theChar == 'c')
    {
      val = va_arg(ap, int);
      do_char_inc((char) val);

      continue;
    } 

#ifdef SUPPORT_LONG
    if(theChar == 'l')
    {
      isLong = 1;
      theChar = *fmtPnt;
      fmtPnt++;
    }
#endif

    if(theChar == 'u') {
      isUnsigned = 1;
    }
    else if(theChar == 'x') {
      base = 16;
    }
    else if(theChar != 'd' && theChar != 'i') {
      do_char_inc(theChar);
      continue;
    }

#ifdef SUPPORT_LONG
    if(isLong)
      val = va_arg(ap, long);
    else
      val = va_arg(ap, int);

    if(isUnsigned && isLong)
      _ultoa(val, buffer, base);
    else if(isUnsigned)
      _uitoa(val, buffer, base);
    else if(isLong)
      _ltoa(val, buffer, base);
    else
      _itoa(val, buffer, base);
#else
    val = va_arg(ap, int);
    
    if(isUnsigned)
      _uitoa(val, buffer, base);
    else
      _itoa(val, buffer, base);
#endif

    strPnt = buffer;
    while((theChar = *strPnt++) != 0) 
      do_char_inc(theChar);
  }

  if(bufPnt) *bufPnt = '\0';

  return count;
}
