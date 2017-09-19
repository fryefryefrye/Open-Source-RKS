/*--------------------------------------------------------------------------
 * bit24lu1.h
 *
 * IAR ICC8051 bit definition file for the Nordic Semiconductor nRF24LU1 and
 * nRF24LU1+ 2.4GHz RF transceiver with embedded 8051 compatible
 * microcontroller.
 *
 *------------------------------------------------------------------------*/
#ifndef BIT24LU1_H
#define BIT24LU1_H

/*  FSR  */
#define MCDIS   FSR_bit.MCDIS
#define STP     FSR_bit.STP
#define WEN     FSR_bit.WEN
#define RDYN    FSR_bit.RDYN
#define INFEN   FSR_bit.INFEN
#define RDIS    FSR_bit.RDIS
#define RDEND   FSR_bit.RDEND
#define WPEN    FSR_bit.WPEN

/*  PSW   */
#define CY      PSW_bit.CY
#define AC      PSW_bit.AC
#define F0      PSW_bit.F0
#define RS1     PSW_bit.RS1
#define RS0     PSW_bit.RS0
#define OV      PSW_bit.OV
#define P       PSW_bit.P

/*  TCON   */
#define TF1     TCON_bit.TF1
#define TR1     TCON_bit.TR1
#define TF0     TCON_bit.TF0
#define TR0     TCON_bit.TR0
#define IE1     TCON_bit.IE1
#define IT1     TCON_bit.IT1
#define IE0     TCON_bit.IE0
#define IT0     TCON_bit.IT0

/*  S0CON  */
#define SM0     S0CON_bit.SM0
#define SM1     S0CON_bit.SM1
#define SM20    S0CON_bit.SM20
#define REN0    S0CON_bit.REN0
#define TB80    S0CON_bit.TB80
#define RB80    S0CON_bit.RB80
#define TI0     S0CON_bit.TI0
#define RI0     S0CON_bit.RI0

/*  T2CON  */
#define T2PS    T2CON_bit.T2PS
#define I3FR    T2CON_bit.I3FR
#define I2FR    T2CON_bit.I2FR
#define T2R1    T2CON_bit.T2R1
#define T2R0    T2CON_bit.T2R0
#define T2CM    T2CON_bit.T2CM
#define T2I1    T2CON_bit.T2I1
#define T2I0    T2CON_bit.T2I0

/*  IEN0  */
#define EA      IEN0_bit.EA
#define ET2     IEN0_bit.ET2
#define ES0     IEN0_bit.ES0 
#define ET1     IEN0_bit.ET1
#define EX1     IEN0_bit.EX1
#define ET0     IEN0_bit.ET0
#define EX0     IEN0_bit.EX0

/* IEN1  */
#define EXEN2   IEN1_bit.EXEN2
#define WUIRQ   IEN1_bit.WUIRQ
#define USB     IEN1_bit.USB
#define USBWU   IEN1_bit.USBWU
#define SPI     IEN1_bit.SPI
#define RF      IEN1_bit.RF
#define RFSPI   IEN1_bit.RFSPI

/* IRCON */
#define EXF2    IRCON_bit.EXF2
#define TF2     IRCON_bit.TF2
#define WUF     IRCON_bit.WUF
#define USBF    IRCON_bit.USBF
#define USBWUF  IRCON_bit.USBWUF
#define SPIF    IRCON_bit.SPIF
#define RFF     IRCON_bit.RFF
#define RFSPIF  IRCON_bit.RFSPIF

/* USBCON */
#define SWRST   USBCON_bit.SWRST
#define WU      USBCON_bit.WU
#define SUSPEND USBCON_bit.SUSPEND
#define IV4     USBCON_bit.IV4
#define IV3     USBCON_bit.IV3
#define IV2     USBCON_bit.IV2
#define IV1     USBCON_bit.IV1
#define IV0     USBCON_bit.IV0

/* PORT0 */
#define P00     P0_bit.P00
#define P01     P0_bit.P01
#define P02     P0_bit.P02
#define P03     P0_bit.P03
#define MCSN    P0_bit.P03
#define SCSN    P0_bit.P03
#define P04     P0_bit.P04
#define P05     P0_bit.P05
#define P06     P0_bit.P06
#define P07     P0_bit.P07

/* PORT1 */
#define P10     P1_bit.P10
#define P11     P1_bit.P11
#define P12     P1_bit.P12
#define P13     P1_bit.P13
#define P14     P1_bit.P14
#define P15     P1_bit.P15
#define P16     P1_bit.P16
#define P17     P1_bit.P17

/* PORT2 */
#define P20     P2_bit.P20
#define P21     P2_bit.P21
#define P22     P2_bit.P22
#define P23     P2_bit.P23
#define P24     P2_bit.P24
#define P25     P2_bit.P25
#define P26     P2_bit.P26
#define P27     P2_bit.P27

/* PORT3 */
#define P30     P3_bit.P30
#define P31     P3_bit.P31
#define P32     P3_bit.P32
#define P33     P3_bit.P33
#define P34     P3_bit.P34
#define P35     P3_bit.P35
#define P36     P3_bit.P36
#define P37     P3_bit.P37

/* RFCON */
#define RFCE    RFCON_bit.RFCE
#define RFCSN   RFCON_bit.RFCSN
#define RFCKEN  RFCON_bit.RFCKEN

/* WDCON */
#define BD      WDCON_bit.BD

/* AESCS */
#define GO      AESCS_bit.GO
#define DECR    AESCS_bit.DECR

/* REGX commands */
#define RWD         0x00
#define WWD         0x08
#define RGTIMER     0x01
#define WGTIMER     0x09
#define RRTCLAT     0x02
#define WRTCLAT     0x0A
#define RRTC        0x03
#define WRTCDIS     0x0B
#define RWSTA0      0x04
#define WWCON0      0x0C
#define RWSTA1      0x05
#define WWCON1      0x0D

#endif
