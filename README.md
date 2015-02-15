tohcom
=============

pseudo-terminal serial-port-over-i2c driver - packed with picocom

start picocom from launcher:

![alt text](https://raw.githubusercontent.com/kimmoli/tohcom/master/picocom/config/tohcom.png "Tohcom icon")

`Tohcom`

picocom instance starts tohcom-daemon-service over dbus, and automatically connects to /dev/pts/ -port created.

picocom changes baud-rate, parity, word-length and stop-bits over dbus. Flow control not yet implemented.

when picocom is exited, tohcom-daemon-service is also stopped.

tohcom can also be run from command-line for testing purposes.

SC16C850L UART datasheet http://www.nxp.com/products/interface_and_connectivity/uarts/single_channel/series/SC16C850L.html

