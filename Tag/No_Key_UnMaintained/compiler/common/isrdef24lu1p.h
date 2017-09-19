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

#define SPI_ISR() \
_Pragma("vector=0x0053") \
__interrupt void spi_isr(void)

#define USB_WU_ISR() \
_Pragma("vector=0x005b") \
__interrupt void usb_wu_isr(void)

#define USB_ISR() \
_Pragma("vector=0x0063") \
__interrupt void usb_isr(void)

#define WU_ISR() \
_Pragma("vector=0x006b") \
__interrupt void wu_isr(void)

#endif /*__ICC8051__*/

#ifdef __C51__

#define EXT_INT0_ISR()  void ext_int0_isr(void) interrupt INTERRUPT_EXT_INT0// External Interrupt0 (P0.3) (0x03)
#define T0_ISR()        void t0_isr(void)       interrupt INTERRUPT_T0      // Timer0 Overflow (0x0b)
#define AES_RDY_ISR()   void aes_rdy_isr(void)  interrupt INTERRUPT_AES_RDY // AES ready interrupt (0x13)
#define T1_ISR()        void t1_isr(void)       interrupt INTERRUPT_T1      // Timer1 Overflow (0x1b)
#define UART0_ISR()     void uart0_isr(void)    interrupt INTERRUPT_UART0   // UART0, Receive & Transmitt interrupt (0x0023)
#define T2_ISR()        void t2_isr(void)       interrupt INTERRUPT_T2      // Timer2 Overflow (0x2b)
#define RF_RDY_ISR()    void rf_rdy_isr(void)   interrupt INTERRUPT_RF_RDY  // RF SPI ready interrupt (0x43)
#define NRF_ISR()       void nrf_isr(void)      interrupt INTERRUPT_RFIRQ   // RF interrupt (0x4b)
#define SPI_ISR()       void spi_isr(void)      interrupt INTERRUPT_SPI     // SPI interrupt (0x53)
#define USB_WU_ISR()    void usb_wu_isr(void)   interrupt INTERRUPT_USB_WU  // USB wakeup interrupt (0x5b)
#define USB_ISR()       void usb_isr(void)      interrupt INTERRUPT_USB_INT // USB interrupt (0x63)
#define WU_ISR()        void wu_isr(void)       interrupt INTERRUPT_WU      // Internal wakeup interrupt (0x6b)

#endif /*__C51__*/




