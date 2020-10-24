# WiFiClock
A WiFi clock who also displays temperature &amp; weather if it turns bad !

Full project details can be found here : https://hackaday.io/project/175515-wifi-clock

This page only shows details about code.

Schematic :

<p align="center">
  <img src="https://romaindurocher.com/MISC/Main_CubeClock_2.0.jpg" alt="schematic">
</p>

<hr>

<h2>Code synoptic</h2>

<p align="center">
  <img src="https://romaindurocher.com/MISC/Main_WiFiClock.jpg" alt="schematic">
</p>

<h2>InterruptVoid()</h2>

When proximity sensor is trigged, it flag an interrupt, who increment a int called <i>interruptHandler</i>, who makes the code enter in <i>interruptVoid()</i> when main loop reach it (and int !=0).

It will loop inside this function until a 5s timer is ended or the sensor is trigged more than five times.
Exiting the <i>interruptVoid()</i> set <i>interruptHandler</i> to 0.

Triggering the sensor will also resetting the timer to 0.

This function displays an information in a list of 5 different, depending the number of times the proximity sensor was triggered. If you hit once, it will display the date during 5s, or if you hit once more, it will display the temperature for 5s ...


<p align="center">
  <img src="https://romaindurocher.com/MISC/interruptVoid().jpg" alt="interruptVoid">
</p>



<hr>

<i>Details in progress !</i>

Some tutorials and code examples I used in this project :

https://randomnerdtutorials.com/esp8266-weather-forecaster/

https://www.arduino.cc/en/Tutorial/DimmingLEDs

https://randomnerdtutorials.com/esp8266-nodemcu-date-time-ntp-client-server-arduino/

https://randomnerdtutorials.com/interrupts-timers-esp8266-arduino-ide-nodemcu/

https://randomnerdtutorials.com/esp8266-nodemcu-date-time-ntp-client-server-arduino/


