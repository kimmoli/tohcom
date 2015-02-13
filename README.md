tohcom
=============

pseudo-terminal serial-port-over-i2c driver - packed with picocom

```tohcom``` needs to be started as root to be able to export gpio

after starting with option ```-m``` it will create /dev/pts/n port which user can connect with included picocom.

Through picocom you can change baudrate of the port, but other parameters are fixed to n,8,1 due ptm limitations.

SC16C850L UART datasheet http://www.nxp.com/products/interface_and_connectivity/uarts/single_channel/series/SC16C850L.html

