# Open-Source-PKE

## What is a PKE system
PKE as the Passive Keyless Entry System. It's a proximity system that is triggered if a key is within a certain distance. <br> 
Widely used in modern vehicles. It's very convenient to Entry your car when you have PKE system. <br> 
You do not need take out of your key, just walk to you car with the key in your pocket, then you can open the door.<br>  
<br> But do you want open the door of your home without take out your keys?<br> 
I've found a solution that every electronic fans can make a PKE system at home in a very cheap price.<br> 
Arduino based Controller can make you to use this system in any application if you like.<br>  
You can submit a issue or mail to me if you have any question.<br> 

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
* Make the Tag/Key/Transmiter

First you need program the nRF24LE1 use a Programmer.<br>
After that, just connect 2 cables from battery to power on the nRF24LE1 module is OK.<br>
All the needles on the module can be cut to short to fit in the thin case.<br>
![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/nRF24LE1_back.jpg)
![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/tag.jpg).<br>
It's not a good way showing on the picture, I have order the PCB for TAG. and it is on the way, I will update soon.


* Make the Controller/Receiver

You can build the Arduino based Controller what ever you like. Just connect nRF24L01 module with Arduino follow the table.<br>

| PIN  | NRF24L01|Arduino UNO/NANO|Mega1280/2560|
| ---------- | -----------| -----------| -----------|
|  1  |  GND  |  GND  |  GND  |
|  2  |  VCC  |  3.3V  |  3.3V  |
|  3  |  CE   |  digIO 7  |  digIO 7  |
|  4  |  CSN  |  digIO 8  |  digIO 8  |
|  5  |  SCK   |  digIO 13  |  digIO 52  |
|  6  |  MOSI  |  digIO 11  |  digIO 51  |
|  7  |  MISO  | digIO 12  |  digIO 50  |
|  8  |  IRQ  |  -  |-|

<br>
This is combination I used in debug stage.<br> 

![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/rx_2560.jpg)
<br> 
![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/nRF24L01_leg.png)
<br>
The PCB for Controller is on the way, I will update soon.<br>

## Technical details

### Frequency hopping

The key, as an Active RFID Tag, will transmit its ID and the volt of the battery 3 times every second in 3 different channels.<br> 
The Controller, as the receiver, will listening on these 3 different channels every second.<br> 
This will make the system more anti-interference.<br> 

### nRF setting



### Power Consumption of the Tag


![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/PowerConsumption.png)<br> 

The above picture is Current sampling with a 47 Ohm resistor.<br> 

For every time of transmit. Power consumption can be estimated as:  <br> 
0.7v\*47Ohm = 32.9mA<br> 
Duration for 0.3ms <br> <br> 
Capacity of a CR2032 battery is 200 mAh.<br> 
In This example, data was transmit 3 tims in every second. <br> 
Estimated life days = (200/(32.9\*(0.0003/3600)))/3/3600/24 = 281 days <br> 



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
