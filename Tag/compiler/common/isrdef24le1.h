#ifndef _ISRDEF24LE1_H_
#define _ISRDEF24LE1_H_

#ifdef __ICC8051__

#define EXT_INT0_ISR() \
_Pragma("vector=0x0003") \
__interrupt void ext_int0_isr(void)

#define T0_ISR() \
_Pragma("vector=0x000b") \
__interrupt void t0_isr(void)

#define AES_RDY_ISR() \
_Pragma("vector=0x0013") \
__interrupt void aes_rdy_isr(void)

#define T1_ISR() \
_Pragma("vector=0x001b") \
__interrupt void t1_isr(void)

#define UART0_ISR() \
_Pragma("vector=0x0023") \
__interrupt void uart0_isr(void)

#define T2_ISR() \
_Pragma("vector=0x002b") \
__interrupt void t2_isr(void)

#define RF_RDY_ISR() \
_Pragma("vector=0x0043") \
__interrupt void rf_rdy_isr(void)

#define NRF_ISR() \
_Pragma("vector=0x004b") \
__interrupt void nrf_isr(void)

#define SER_ISR() \
_Pragma("vector=0x0053") \
__interrupt void serial_isr(void)

#define WUOP_ISR() \
_Pragma("vector=0x005b") \
__interrupt void wuop_isr(void)

#define MISC_ISR() \
_Pragma("vector=0x0063") \
__interrupt void misc_isr(void)

#define ADC_ISR() \
_Pragma("vector=0x0063") \
__interrupt void adc_isr(void)

#define TICK_ISR() \
_Pragma("vector=0x006b") \
__interrupt void tick_isr(void)

#endif /*__ICC8051__*/

#ifdef __C51__

#define EXT_INT0_ISR()  void ext_int0_isr(void) interrupt INTERRUPT_IPF     // External Interrupt0 (P0.3) (0x03)
#define T0_ISR()        void t0_isr(void)       interrupt INTERRUPT_T0      // Timer0 Overflow (0x0b)
#define AES_RDY_ISR()   void aes_rdy_isr(void)  interrupt INTERRUPT_POFIRQ  // Power failure interrupt (0x13)
#define T1_ISR()        void t1_isr(void)       interrupt INTERRUPT_T1      // Timer1 Overflow (0x1b)
#define UART0_ISR()     void uart0_isr(void)    interrupt INTERRUPT_UART0   // UART0, Receive & Transmitt interrupt (0x0023)
#define T2_ISR()        void t2_isr(void)       interrupt INTERRUPT_T2      // Timer2 Overflow (0x2b)
#define RF_RDY_ISR()    void rf_rdy_isr(void)   interrupt INTERRUPT_RFRDY   // RF SPI ready interrupt (0x43)
#define NRF_ISR()       void nrf_isr(void)      interrupt INTERRUPT_RFIRQ   // RF interrupt (0x4b)
#define SER_ISR()       void serial_isr(void)   interrupt INTERRUPT_SERIAL  // SERIAL / SPI interrupt (0x53)
#define WUOP_ISR()      void wuop_isr(void)     interrupt INTERRUPT_WUOPIRQ // Wake on pin interrupt (0x5b)
#define MISC_ISR()      void misc_isr(void)     interrupt INTERRUPT_MISCIRQ // MISC interrupt (0x63)
#define ADC_ISR()       void adc_isr(void)      interrupt INTERRUPT_MISCIRQ // ADC interrupt (0x63)
#define TICK_ISR()      void tick_isr(void)     interrupt INTERRUPT_TICK    // Internal wakeup interrupt (0x6b)

#endif /*__C51__*/

#endif




