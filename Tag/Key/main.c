#include <reg24le1.h>
#include <stdint.h>
#include <stdbool.h>
#include "hal_nrf.h"
#include "hal_nrf_hw.h"
#include "hal_clk.h"
#include "hal_rtc.h"
#include "hal_delay.h"
#include "hal_adc.h"
#include "hal_wdog.h"
#include "hal_uart.h"
#include "string.h"


/*************************user modify settings****************************/
#define  TAG_ID 2

uint8_t  TX_ADDRESS[5]  = {0x55,0x56,0x57,0x58,0x59}; // TX address
uint8_t HopCH[3] = {105,76,108};

#define  TAG_TIME        10530+64*TAG_ID		//sleep time£º10530=0.3s;	32768=1s
#define  TX_PAYLOAD  	 4        	// data length
#define  ADC_TIME        10800		//volt of BAT,   every =   ADC_TIME*TAG_TIME     

#define  RF_POWER 		 HAL_NRF_6DBM
//HAL_NRF_18DBM,          /**< Output power set to -18dBm */
//HAL_NRF_12DBM,          /**< Output power set to -12dBm */
//HAL_NRF_6DBM,           /**< Output power set to -6dBm  */
//HAL_NRF_0DBM            /**< Output power set to 0dBm   */

/******************************************************************************/


//#define  S1    P15  // Key S1
//#define  S2    P16  // Key S2



/* Watchdog*/
#define USE_WDT   1
#define WDT_TIME  256  //2S



uint8_t   CellVoltageH;
uint8_t   CellVoltageL;
xdata bool  radio_busy;
xdata uint8_t  TxPayload[TX_PAYLOAD];
xdata uint8_t  RxPayload[32];
uint16_t PipeAndLen;
uint8_t CurrCH = 0;


void PrintInt16(uint16_t Counter);


void UART0_SendStr( char* str )
{
    uint8_t i;
    uint8_t size = strlen(str);
    for(i=0; i<size; i++)
    {
        hal_uart_putchar(*(str+i));
    }
}

void IoInit(void)
{
    //P0DIR = 0x00;
    //P1DIR = 0x00;

	P1DIR = ( 1<<5 ) | ( 1<<6 );    //p0.0 p0.1  input£¬others output
	P0DIR = 0;
	P1 = 0xff;
	P1CON = 0xD5;                   //Enable pull up for P0.0
	P1CON = 0xD6;                   //Enable pull up for P0.1
	P0 &= ~( 1<<2 );
	P0 &= ~( 1<<1 );  

}


void adc_init(void)		 //get volt of BAT
{
    hal_adc_set_input_channel(HAL_INP_VDD1_3);          //1/3 VDD
    hal_adc_set_reference(HAL_ADC_REF_INT);             //set REF int 1.2V
    hal_adc_set_input_mode(HAL_ADC_SINGLE);
    hal_adc_set_conversion_mode(HAL_ADC_SINGLE_STEP);
    hal_adc_set_sampling_rate(HAL_ADC_2KSPS);
    hal_adc_set_resolution(HAL_ADC_RES_12BIT);          //12 bit ADC
    hal_adc_set_data_just(HAL_ADC_JUST_RIGHT);
}

void set_timer_period(uint16_t period)
{
    if((period<10) && (period>65536))period = 32768;
    hal_rtc_start(false);
    hal_rtc_start(true);
    hal_rtc_set_compare_value(period - 1);
}


void RfCofig(void)
{
    RFCKEN = 1;

    hal_nrf_close_pipe(HAL_NRF_ALL);
    hal_nrf_open_pipe(HAL_NRF_PIPE0,false);

    hal_nrf_set_operation_mode(HAL_NRF_PTX);
    hal_nrf_set_rf_channel(HopCH[0]);
    hal_nrf_set_datarate(HAL_NRF_2MBPS);
    hal_nrf_set_output_power(RF_POWER);
    hal_nrf_set_crc_mode(HAL_NRF_CRC_8BIT);
    hal_nrf_set_address_width(HAL_NRF_AW_5BYTES);
    hal_nrf_enable_dynamic_payload(false);
    hal_nrf_set_rx_payload_width(0,TX_PAYLOAD);
    hal_nrf_set_address(HAL_NRF_TX,TX_ADDRESS);
    hal_nrf_set_auto_retr(0,1500);
    hal_nrf_set_power_mode(HAL_NRF_PWR_UP);
    RF = 1;
    EA = 1;
}

void mcu_init(void)
{
    hal_rtc_start(false);
    hal_clklf_set_source(HAL_CLKLF_RCOSC32K);           // Use 32.768KHz RC

    hal_rtc_set_compare_mode(HAL_RTC_COMPARE_MODE_0);   // Use 32 KHz timer mode 0
    set_timer_period(TAG_TIME);	                        // Set the RTC2 time£¬card sleep time
    hal_clk_set_16m_source(HAL_CLK_XOSC16M);            // Always run on 16MHz crystal oscillator
    hal_clk_regret_xosc16m_on(0);                       // Keep XOSC16M off in register retention

    hal_rtc_start(true);

    while((CLKLFCTRL&0x80)==0x80);	                    // Wait for the 32kHz to startup (change phase)
    while((CLKLFCTRL&0x80)!=0x80);
}

void Assemble_Data(void)
{

	P1 |= ( 1<<5 ) | ( 1<<6 );  //P1.5 = 1; P1.6 = 1
	if( !( P1 & ( 1<<5 ) ) )    //check P1.5
	{
		TxPayload[0] = 2;//FUNCTION_CODE
	}
	else if( !( P1 & ( 1<<6 ) ) )    //Check P1.6
	{
		TxPayload[0] = 3;//FUNCTION_CODE
	}
	else
	{
		TxPayload[0] = 0;//FUNCTION_CODE
	}

    TxPayload[1] = TAG_ID;
    TxPayload[2] = CellVoltageH;
    TxPayload[3] = CellVoltageL;
}

void main()
{

    xdata   uint32_t  loopCount = ADC_TIME-1;

    IoInit();
    mcu_init();

    //hal_uart_init(UART_BAUD_9K6);  //UART£¬bps9600
    while(hal_clk_get_16m_source() != HAL_CLK_XOSC16M)
    {
    }

    //hal_uart_putchar('a');

    adc_init();
    RfCofig();

#ifdef  USE_WDT
    hal_wdog_init(WDT_TIME);//watch dog 2s
#endif

    while(1)
    {
        loopCount++;
        //PrintInt16(loopCount);
        //UART0_SendStr("\r\n");

#ifdef  USE_WDT
        hal_wdog_restart(); //feed dog
#endif

        if(loopCount == ADC_TIME)    //Get volt of BAT every 5mins
        {
            hal_adc_start();           //ADC
            while( hal_adc_busy())     //wait ADC finish
                ;
            CellVoltageH = hal_adc_read_MSB(); //get ADC
            CellVoltageL = hal_adc_read_LSB();
            loopCount=0;
        }

        CurrCH++;
        if (CurrCH>2)
        {
            CurrCH = 0;
        }
        hal_nrf_set_rf_channel(HopCH[CurrCH]);	  //sent in 3 channels in one senconds



        PWRDWN = 0x04;    // power down
        PWRDWN = 0x00;

        Assemble_Data();
        hal_nrf_write_tx_payload_noack(TxPayload,TX_PAYLOAD);

        CE_PULSE();	            //RF send
        radio_busy = true;
        while(radio_busy)		    //wait RF finish
            ;
    }
}

void rf_irq() interrupt INTERRUPT_RFIRQ
{
    uint8_t  irq_flags;

    irq_flags = hal_nrf_get_clear_irq_flags();

    if(irq_flags & (1<<HAL_NRF_RX_DR))
    {

        while(!hal_nrf_rx_fifo_empty())// Read payload
        {
            PipeAndLen = hal_nrf_read_rx_payload(RxPayload);
        }
        radio_busy = false;
    }

    if(irq_flags & ((1<<HAL_NRF_TX_DS)))			// transimmter finish
    {
        radio_busy = false;
    }

    if(irq_flags & ((1<<HAL_NRF_MAX_RT)))			// re-transimmter
    {
        radio_busy = false;
        hal_nrf_flush_tx();
    }
}

//void PrintInt16(uint16_t Counter)
//{
//    uint8_t onechar;
//
//    onechar = Counter/10000%10+0x30;
//    hal_uart_putchar(onechar);
//
//    onechar = Counter/1000%10+0x30;
//    hal_uart_putchar(onechar);
//
//    onechar = Counter/100%10+0x30;
//    hal_uart_putchar(onechar);
//
//    onechar = Counter/10%10+0x30;
//    hal_uart_putchar(onechar);
//
//    onechar = Counter%10+0x30;
//    hal_uart_putchar(onechar);
//}
