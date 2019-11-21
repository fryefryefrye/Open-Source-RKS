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
#define  TAG_ID 8


#define  TAG_TIME				10530+64*TAG_ID		//sleep time£º10530=0.3s;	32768=1s
#define  ADC_TIME				10800		//volt of BAT,   every =   ADC_TIME*TAG_TIME
#define  TX_PAYLOAD_LEN			6        	// data length
#define  TX_PACKET_LEN			TX_PAYLOAD_LEN + 16        	// data length
#define  TX_CH_NUMBER			3        	// data length       

#define  RF_POWER 		 HAL_NRF_0DBM
//HAL_NRF_18DBM,          /**< Output power set to -18dBm */
//HAL_NRF_12DBM,          /**< Output power set to -12dBm */
//HAL_NRF_6DBM,           /**< Output power set to -6dBm  */
//HAL_NRF_0DBM            /**< Output power set to 0dBm   */

/******************************************************************************/


//#define  S1    P15  // Key S1
//#define  S2    P16  // Key S2

//// The MAC address of the beacon
#define MY_MAC_0  0x60
#define MY_MAC_1  0x59
#define MY_MAC_2  0x48
#define MY_MAC_3  0x47
#define MY_MAC_4  0x46
#define MY_MAC_5  0x45

#define ANDROID		 0x42
#define IPHONE		0x40

/* Watchdog*/
#define USE_WDT   1
#define WDT_TIME  256  //2S


uint8_t   CellVoltageH;
uint8_t   CellVoltageL;
xdata bool  radio_busy;
xdata uint8_t  TxPayload[TX_PAYLOAD_LEN];
uint16_t PipeAndLen;
uint8_t CurrCH = 0;


void PrintInt16(uint16_t Counter);
void UART0_SendStr( char* str );

//xdata uint8_t _packet[TX_PACKET_LEN] = {0xBA,0x1C,0x54,0x60,0xBB,0x85,0x53,0xF3,0xC3,0xEB,0x03,0x82,0xFB,0xBA,0xBB,0xC7,0x71,0x7E,0x2D,0x01}; //maximum size of payload handled by nrf24L01 is 32
xdata uint8_t _packet_coded[TX_CH_NUMBER][TX_PACKET_LEN] = {{0xF3,0xBB,0xEC,0x1F,0xA6,0x0F,0x0C,0xA7,0xEE,0x0C,0x28,0xB2,0x96,0x6E,0x5F,0x07,0x42,0x1F,0x82,0x85}  //Len = 20 ch = 0Start LE advertizing
,{0x29,0x53,0x24,0x9E,0x80,0x91,0xED,0x5B,0x98,0x25,0x55,0xE2,0x21,0xF2,0xF3,0xC6,0xF7,0xCE,0x13,0x87}  //Len = 20 ch = 1Start LE advertizing
,{0xBA,0x1C,0x54,0x60,0xBB,0x85,0x53,0xF3,0xC3,0xEB,0x03,0x82,0xFB,0x1A,0x3B,0x47,0xD1,0x50,0xF2,0x7B}};  //Len = 20 ch = 2Start LE advertizing

uint8_t _length = 0; //length of packet filled
uint8_t _dataFieldStartPoint = 0;
xdata uint8_t chRf[] = {2,26,80} ; 
xdata uint8_t chLe[] = {37,38,39} ;

#define crc_mul 0x1021
uint16_t cal_crc(unsigned char *ptr, unsigned char len)
{
	unsigned char i;
	unsigned int crc=0;
	while(len-- != 0)
	{
		for(i=0x80; i!=0; i>>=1)
		{
			if((crc&0x8000)!=0)
			{
				crc<<=1;
				crc^=(crc_mul);
			}else{
				crc<<=1;
			}
			if((*ptr&i)!=0)
			{
				crc ^= (crc_mul);
			}
		}
		ptr ++;
	}
//	UART0_SendStr("crc:");
//	PrintInt16(crc);
//	UART0_SendStr("\r\n");
	return (crc);
}

void fill_crc(unsigned char *ptr, unsigned char stru_len)
{
	uint16_t * pSum = ((uint32_t*)(ptr+stru_len-2));
	*pSum = cal_crc(ptr,stru_len-2);
}

uint8_t  reverseBits(uint8_t input)
{
		// reverse the bit order in a single byte
		uint8_t temp = 0;
		if (input & 0x80) temp |= 0x01;
		if (input & 0x40) temp |= 0x02;
		if (input & 0x20) temp |= 0x04;
		if (input & 0x10) temp |= 0x08;
		if (input & 0x08) temp |= 0x10;
		if (input & 0x04) temp |= 0x20;
		if (input & 0x02) temp |= 0x40;
		if (input & 0x01) temp |= 0x80;
		return temp;
}
uint8_t  bleWhitenStart(uint8_t chan){
	//use left shifted one
	return reverseBits(chan) | 2;
}

void setPhone(uint8_t phone_type)
{
	//byte no.0 PDU
	uint8_t i;
	for(i=0; i<TX_CH_NUMBER; i++)
	{
		_packet_coded[i][0] = phone_type;	
	}
}
void setMAC(uint8_t m0, uint8_t m1, uint8_t m2, uint8_t m3, uint8_t m4, uint8_t m5)
{
	uint8_t i;
	for(i=0; i<TX_CH_NUMBER; i++)
	{
		//length of payload is entered in byte no.1
		_length = 2;
		_packet_coded[i][_length++] = m0;
		_packet_coded[i][_length++] = m1;
		_packet_coded[i][_length++] = m2;
		_packet_coded[i][_length++] = m3;
		_packet_coded[i][_length++] = m4;
		_packet_coded[i][_length++] = m5;
		//length should be 8 by now
		//flags (LE-only, limited discovery mode)
		_packet_coded[i][_length++] = 2;    //flag length
		_packet_coded[i][_length++] = 0x01; //data type
		_packet_coded[i][_length++] = 0x05; //actual flag
	}
}

void setData(void* Data,uint8_t dataLen)
{
	uint8_t i,j;
	uint8_t* current;
	_dataFieldStartPoint = (_dataFieldStartPoint==0?_length:_dataFieldStartPoint);


	for(j=0; j<TX_CH_NUMBER; j++)
	{
		_length = _dataFieldStartPoint;
		current = (uint8_t*)(Data);
	
		_packet_coded[j][_length++] = dataLen +1;
		_packet_coded[j][_length++] = 0xFF;//data type
		for (i = 0; i < dataLen; i++){
			_packet_coded[j][_length++] = *(current);
			current++;
		}
		//CRC is appended to the data
		//CRC starting val 0x555555 acc. to spec
		_packet_coded[j][_length++] = 0x55;
		_packet_coded[j][_length++] = 0x55;
		_packet_coded[j][_length++] = 0x55;
	}
}

void BLEcrc(const uint8_t* Data, uint8_t dataLen, uint8_t* outputCRC){
	// calculating the CRC based on a LFSR
	uint8_t i, temp, tempData;

	while (dataLen--){
		tempData = *Data++;
		for (i = 0; i < 8; i++, tempData >>= 1){
			temp = outputCRC[0] >> 7;
			
			outputCRC[0] <<= 1;
			if (outputCRC[1] & 0x80){ outputCRC[0] |= 1; }
			outputCRC[1] <<= 1;
			if (outputCRC[2] & 0x80){ outputCRC[1] |= 1; }
			outputCRC[2] <<= 1;

			if (temp != (tempData & 1)){
				outputCRC[2] ^= 0x5B;
				outputCRC[1] ^= 0x06;
			}
		}
	}
}

void bleWhiten(uint8_t* Data, uint8_t len, uint8_t whitenCoeff){
	// Implementing whitening with LFSR
	uint8_t  m;
	while (len--){
		for (m = 1; m; m <<= 1){
			if (whitenCoeff & 0x80){
				whitenCoeff ^= 0x11;
				(*Data) ^= m;
			}
			whitenCoeff <<= 1;
		}
		Data++;
	}
}

void blePacketEncode(uint8_t* packet, uint8_t len, uint8_t chan){
	// Assemble the packet to be transmitted
	// Packet length includes pre-populated crc 
	uint8_t i, dataLen = len - 3;
	BLEcrc(packet, dataLen, packet + dataLen);
	for (i = 0; i < 3; i++, dataLen++)
		packet[dataLen] = reverseBits(packet[dataLen]);
	bleWhiten(packet, len, bleWhitenStart(chan));
	for (i = 0; i < len; i++)
		packet[i] = reverseBits(packet[i]); // the byte order of the packet should be reversed as well
}



void sendADV(uint8_t channel)
{
	
	hal_nrf_set_rf_channel(chRf[channel]);//_radio.setChannel(RF24BLE::chRf[channel]);

//	UART0_SendStr("Tx Len:");
//	PrintInt16(_length);
//	UART0_SendStr("\r\n");
//
//	UART0_SendStr("tx channel");
//	PrintInt16(channel);
//	UART0_SendStr("\r\n");

	if(_length <= TX_PACKET_LEN)
	{
		hal_nrf_write_tx_payload_noack(_packet_coded[channel],_length);//_radio.startWrite(_packet, _length,false);
	}
	else
	{
		UART0_SendStr("_length > TX_PACKET_LEN !\r\n");
	}
}


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
	uint8_t  TX_ADDRESS[4]  = {0x8e,0x89,0xBE,0xD6};
	TX_ADDRESS[0] = reverseBits(TX_ADDRESS[0]);
	TX_ADDRESS[1] = reverseBits(TX_ADDRESS[1]);
	TX_ADDRESS[2] = reverseBits(TX_ADDRESS[2]);
	TX_ADDRESS[3] = reverseBits(TX_ADDRESS[3]);

    RFCKEN = 1;


    //BLE
	hal_nrf_close_pipe(HAL_NRF_ALL);
	hal_nrf_open_pipe(HAL_NRF_PIPE0,false);//	_radio.setAutoAck(false);
	hal_nrf_set_operation_mode(HAL_NRF_PTX);
	hal_nrf_set_rf_channel(chRf[0]);
	hal_nrf_set_datarate(HAL_NRF_1MBPS);//	_radio.setDataRate(RF24_1MBPS);
	hal_nrf_set_output_power(RF_POWER);//	_radio.setPALevel(RF24_PA_MAX);
	hal_nrf_set_crc_mode(HAL_NRF_CRC_OFF);//	_radio.disableCRC();
	hal_nrf_set_address_width(HAL_NRF_AW_4BYTES);//	_radio.setAddressWidth(4);
	hal_nrf_enable_dynamic_payload(false);
	hal_nrf_set_address(HAL_NRF_TX,TX_ADDRESS);//	_radio.openWritingPipe(address);
	hal_nrf_set_auto_retr(0,0);//	_radio.setRetries(0, 0);	
	hal_nrf_set_power_mode(HAL_NRF_PWR_UP);//	_radio.powerUp();

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
	uint8_t i;

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

	fill_crc(TxPayload,TX_PAYLOAD_LEN);

	setPhone(ANDROID);
	setMAC(MY_MAC_0, MY_MAC_1, MY_MAC_2, MY_MAC_3, MY_MAC_4, MY_MAC_5);
		
	setData(TxPayload, TX_PAYLOAD_LEN);

	for(i=0; i<TX_CH_NUMBER; i++)
	{
		_packet_coded[i][1] = _length-5;//subtract checksum bytes and the 2 bytes including the length byte and the first byte
		blePacketEncode(_packet_coded[i], _length, chLe[i]);
	}

//	UART0_SendStr("Assemble Len:");
//	PrintInt16(_length);
//	UART0_SendStr("\r\n");

}

void main()
{
    xdata   uint32_t  loopCount = ADC_TIME-1;

    IoInit();
    mcu_init();

    hal_uart_init(UART_BAUD_9K6);  //UART£¬bps9600
    while(hal_clk_get_16m_source() != HAL_CLK_XOSC16M)
    {
    }

    hal_uart_putchar('a');

    adc_init();
    RfCofig();

#ifdef  USE_WDT
    hal_wdog_init(WDT_TIME);//watch dog 2s
#endif

    while(1)
    {
        loopCount++;

//        PrintInt16(loopCount);
//        UART0_SendStr("\r\n");

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
			Assemble_Data();
        }

        CurrCH++;
        if (CurrCH>2)
        {
            CurrCH = 0;
        }

//		UART0_SendStr("channel");
//		PrintInt16(CurrCH);
//		UART0_SendStr("\r\n");


        PWRDWN = 0x04;    // power down
        PWRDWN = 0x00;


	  	sendADV(CurrCH);


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
//            PipeAndLen = hal_nrf_read_rx_payload(RxPayload);
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

void PrintInt16(uint16_t Counter)
{
    uint8_t onechar;

    onechar = Counter/10000%10+0x30;
    hal_uart_putchar(onechar);

    onechar = Counter/1000%10+0x30;
    hal_uart_putchar(onechar);

    onechar = Counter/100%10+0x30;
    hal_uart_putchar(onechar);

    onechar = Counter/10%10+0x30;
    hal_uart_putchar(onechar);

    onechar = Counter%10+0x30;
    hal_uart_putchar(onechar);
}
