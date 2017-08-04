# Open-Source-PKE

It's very convenient to Entry your car when you have PKE system.<br>  
But you still have to take out your key to open the home door.<br>  
Do you want open the door. without take out your keys.<br> 
Let's make your own PKE system for your home.<br>  

# A very simple wireless Key. 

`nRF24LE1` module + `CR2032` battery can make a key for PKE system. <br>
nRF24LE1 is Ultra-low Power Wireless System On-Chip Solution.<br>
An 8051 compatible MCU is attached in it. So one chip is enouogh.<br>
![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/tag.jpg)


# Arduino based controler

An arduino equipment with nRF24L01 module, can recived the single from the key near by, after check the ID of the key, it will driver the relay to Unlock the door for you.<br> 
When the key is get a little far away, the relay will release and the door will be locked.<br> 
You can also equipment with some power system to open the door automaticly.<br> 
Arduino based controler is very easy to program, and you can make any application if you wish.
![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/rx_simple.jpg).<br> 



## Make the Tag/Key

Just connect 2 cables to power on the nRF24LE1 module is OK.<br>
![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/nRF24LE1_back.jpg)
![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/tag_simple.jpg).<br>
The PCB for TAG is on the way, I will update soon.

## Make the Controler/Reciver

You can build the Arduino based controler what ever you like. Just connect nRF24L01 module with Arduino via SPI port.<br>
RF24 table.<br>
![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/rx_line.jpg).<br> 
The PCB for Controler is on the way, I will update soon.<br>

### Communication parameters of nRF modules

# Frequency hopping

The key, as an Active RFID Tag, will tranmit its ID and the volt of the battray 3 times every sencond in 3 different channels.<br> 
The controler, as the reciver, will listening on these 3 different channels every sencond.<br> 
This will make the system more anti-interference.<br> 

#### Hardware used in this project

##### Video

###### Title

![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/nRF24LE1.jpg).<br> 
![](https://github.com/fryefryefrye/Open-Source-PKE/raw/master/img/nRF24LE1_back.jpg)
