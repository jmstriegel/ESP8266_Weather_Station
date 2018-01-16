# ESP8266_Weather_Station

Note: Work in progress. This is an insecure proof-of-concept.

An Arduino application and web dashboard for monitoring
weather and air quality. This is designed for use with the Adafruit 
Feather HUZZAH ESP8266, the Plantower PMS7003 sensor, and
Google Appengine's free tier.

## Feather HUZZAH ESP8266

This microcontroller was chosen for its low cost and built in WiFi.

[Adafruit Feather HUZZAH ESP8266](https://www.adafruit.com/product/2821)

## PMS7003

The PMS7003 is a particle matter counter capable of detecting
particulates in the range of 0.3 to 10 microns.

[Plantower_PMS7003 Arduino Library](https://github.com/jmstriegel/Plantower_PMS7003)

This device uses a serial connection to send measurements to a
connected device roughly once a second. Concentration measurements
are provided for PM1.0 (0.3-1.0um), PM2.5 (1.0-2.5um), and PM10
(2.5-10.0um).

## Temperature / Barometer / Humidity

Not yet implemented. More to come...

## Google Appengine

I'm trying to design this to work within Appengine's free service tier.
Unfortunately, this rules out some data storage options that are more amenable
to time series data, so expect some experimentation here. More to come...


## TODO

  - Security restrictions for device access
  - Installation and configuration isntructions
  - Investigate multiple station support limitations within free appengine tier
  - Storage, filtering and retrieval of historical data



## License

This library was written by Jason Striegel and provided
under the BSD license. See license.txt for details.

