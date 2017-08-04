# Open-Source-PKE

It's very convenient to Entry your car when you have PKE system.<br>  
Do you want open the door of your home without take out your keys.<br> 
Let's make your own PKE system for your home or other application if you like.<br>  

## A very simple wireless Key. 

nRF24LE1 module and CR2032 battery are the only thing to make a key for PKE system. <br>
nRF24LE1 is Ultra-low Power Wireless System On-Chip Solution.<br>
An 8051 compatible MCU is attached in it. So one chip is enouogh.<br>
![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/tag.jpg)


## Arduino based controler

An arduino equipment with nRF24L01 module, can recived the data from the key near by.<br> 
Arduino based controler is very easy to program, and you can make any application if you wish.<br> 
In This example, a relay will be used as the output. <br> 
When key get closed, the relay closed.<br> 
When key get far away, the relay will be open.<br> 
![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/rx_simple.jpg).


## How them made?
### Make the Tag/Key

First you need program the nRF24LE1 use a prgramer.
Just connect 2 cables from battery to power on the nRF24LE1 module is OK.<br>
![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/nRF24LE1_back.jpg)
![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/tag_simple.jpg).<br>
It's not a good way showing on the picture, I have order the PCB for TAG. and it is on the way, I will update soon.


### Make the Controler/Reciver

You can build the Arduino based controler what ever you like. Just connect nRF24L01 module with Arduino via SPI port.<br>
RF24 table.<br>
This is combination I used in debug stage.
![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/rx_2560.jpg).<br> 
The PCB for Controler is on the way, I will update soon.<br>

### Communication parameters of nRF modules

#### Frequency hopping

The key, as an Active RFID Tag, will tranmit its ID and the volt of the battray 3 times every sencond in 3 different channels.<br> 
The controler, as the reciver, will listening on these 3 different channels every sencond.<br> 
This will make the system more anti-interference.<br> 

#### nRF setting

#### Power Consumption of the Tag

## Hardware used in this project

### nRF24LE1 module

Price € 4.00<br> 
![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/nRF24LE1.jpg)<br> 

### nRF24L01 module

Price € 2.00<br> 
![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/nRF24L01.jpg)<br> 

### nRF24LE1 use a prgramer.

Price € 15.00<br> 
![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/prgramer.jpg)<br> 

### nRF24LE1 adapter.

Price € 5.00<br> 
![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/adapter.jpg)<br> 

### CR2032 battery
### Arduino

## Video

## Title

after check the ID of the key, it will driver the relay to Unlock the door for you.<br> 
When the key is get a little far away, the relay will release and the door will be locked.<br> 
You can also equipment with some power system to open the door automaticly.<br> 

![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/nRF24LE1.jpg).<br> 
![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/nRF24LE1_back.jpg)
![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/rx_2560.jpg)


picture_back+-
2560+module