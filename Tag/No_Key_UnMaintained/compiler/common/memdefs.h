#ifndef MEMDEFS_H_
#define MEMDEFS_H_

#ifdef __C51__
#define __no_init
#endif /*__C51__*/

#ifdef __ICC8051__

#define idata __idata
#define xdata __xdata
#define code __code
#define data __data
#define pdata __pdata
#define _at_ @
#define _nop_() __no_operation()
#define bit __bit
__sfr __no_init volatile unsigned int CRC @ 0xCA;
__sfr __no_init volatile unsigned int T2  @ 0xCC;

#define reentrant
#if __DATA_MODEL__ == __DATA_MODEL_LARGE__
  #warning "if not using large data model: reentrant procedures must be declared using IAR syntax"
#endif /*__DATA_MODEL__ == __DATA_MODEL_LARGE__*/

#define CLKCTRL CLKFCTL

#ifndef NULL
#define NULL _NULL
#endif /*NULL*/

#endif /*__ICC8051__*/

#endif /*MEMDEFS_H_*/
