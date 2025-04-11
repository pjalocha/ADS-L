A demo reception for ADS-L Low Data Rate on the O-Band.

It is setup to work on a TTGO module with or without GPS
and has been so far tested with SX1276 RF chip.

If you have PlatformIO set up (command-line is enough) the you compile and upload like this:

pio run -e ttgo-sx1276-tbeam -t upload --upload-port=/dev/ttyUSB0 && minicom -D /dev/ttyUSB0

This pairs with the demo transmitter code in Tx-demo which sends M-Band and O-Band packets sequentially on all channels
