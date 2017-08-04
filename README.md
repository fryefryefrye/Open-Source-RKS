# Open-Source-PKE

It's very convenient to Entry your car when you have PKE system.<br>  
Do you want open the door of your home without take out your keys.<br> 
Let's make your own PKE system for your home or other application if you like.<br>  

## A very simple wireless Key. 

nRF24LE1 module and CR2032 battery are the only thing to make a key for PKE system. <br>
nRF24LE1 is Ultra-low Power Wireless System On-Chip Solution.<br>
An 8051 compatible MCU is attached in it. So one chip is enough.<br>
![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/tag_simple.jpg)


## Arduino based Controller

An Arduino equipment with nRF24L01 module, can received the data from the key nearby.<br> 
Arduino based Controller is very easy to program, and you can make any application if you wish base on this PKE system.<br> 
In This example, a relay will be used as the output. <br> 
When key goes close in about 2 meters, the relay closed.<br> 
When key goes far away, the relay will be disconnect.<br> 
![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/rx_simple.jpg).


## How them made?
### Make the Tag/Key

First you need program the nRF24LE1 use a Programmer.<br>
After that, just connect 2 cables from battery to power on the nRF24LE1 module is OK.<br>
All the needles on the module can be cut to short to fit in the thin case.<br>
![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/nRF24LE1_back.jpg)
![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/tag.jpg).<br>
It's not a good way showing on the picture, I have order the PCB for TAG. and it is on the way, I will update soon.


### Make the Controller/Recover

You can build the Arduino based Controller what ever you like. Just connect nRF24L01 module with Arduino via SPI port.<br>

| PIN  | NRF24L01|Arduino UNO|
| ---------- | -----------| -----------|
|  1  |  GND  |  GND  |
|  2  |  VCC  |  3.3V  |
|  3  |  CE   |  digIO 7  |
|  4  |  CSN  |  digIO 8  |
|  5  |  CSN  |  digIO 13  |
|  6  |  SCK  |  digIO 11  |
|  7  |  MOSI  | digIO 12  |
|  8  |  MISO  |  -  |

<br>
This is combination I used in debug stage.<br> 
![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/rx_2560.jpg).<br> 
The PCB for Controller is on the way, I will update soon.<br>

### Communication parameters of nRF modules

#### Frequency hopping

The key, as an Active RFID Tag, will transmit its ID and the volt of the battery 3 times every second in 3 different channels.<br> 
The Controller, as the receiver, will listening on these 3 different channels every second.<br> 
This will make the system more anti-interference.<br> 

#### nRF setting

#### Power Consumption of the Tag

![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/PowerConsumption.png)<br> 

## Hardware used in this project

### nRF24LE1 module

Price € 4.00<br> 
![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/nRF24LE1.jpg)<br> 

### nRF24L01 module

Price € 2.00<br> 
![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/nRF24L01.jpg)<br> 

### Programmer for nRF24LE1

Price € 15.00<br> 
![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/programer.jpg)<br> 

### nRF24LE1 adapter.

Price € 5.00<br> 
![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/adapter.jpg)<br> 

### CR2032 battery
### Arduino

## Video

## Draft

Title

after check the ID of the key, it will drive the relay to Unlock the door for you.<br> 
When the key is get a little far away, the relay will release and the door will be locked.<br> 
You can also equipment with some power system to open the door automatically.<br>
