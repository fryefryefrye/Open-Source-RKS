

//ESP8266启动后，读取继电器状态 和 刚启动标志。
//启动多时的话，以此为准。刚启动的话，从服务器读取状态。



struct tIicCommand
{
	//byte OperationCode;

	//0-8 RelayState
	byte RelayState;

	//0-8 RelayNeedSet
	byte RelayNeedSet;
	byte Rsv;
	byte Rsv2;
	unsigned long Sum;
};


struct tIicData
{
	//0-4 Relay 
	//5:Start more than 30s 
	byte RelayState;

	//0-8 TagOnline
	byte TagState;
	byte Rsv;
	byte Rsv2;
	unsigned long Sum;
};

#define crc_mul 0x1021  //生成多项式
unsigned long cal_crc(unsigned char *ptr, unsigned char len)
{
	unsigned char i;
	unsigned long crc=0;
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
	return (crc);
}
