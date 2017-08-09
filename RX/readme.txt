This is an arduino project.

It can be download to:
Arduino 2560
Arduino nano
Arduino uno
......


RX.ino

It's a very basic application. If it get data from Tag, the relay of the door will closed.



RX_315.ino

It's a bit advanced application. If it get data from Tag, it will send out remote control single like your car key, to Ulock or Lock your car.
A RF_TX module of 433/315 Mhz is needed. Also you must know protocol of your car key.



rx_web.ino

It's another bit advanced application. It works like the first door controller, but it will record all entry log with the  volt of the battery.
With a ESP8266, it also works as a web server, you can open the address of the controller, check the logs.