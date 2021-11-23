# FatFs sdmmc with hardware interface

FatFs sdmmc adapted to use a hardware interface, allowing an easy integration of different hardware platforms. Uses SPI for communication and optional RTC for timestamps, modified from "generic" version of sample projects downloaded through elm-chan's website:

http://elm-chan.org/fsw/ff/00index_e.html

The necessary files to include in any hardware projects are inside the folder named "_ff14b_".

You need to create a interface (source file) to adapt the hardware's interface to the SD, overriding the _default.c_ located in "_ff14b/hwInterface_". The functions that you need to provide are:

* is_initialized
* select
* deselect
* spi_transmit_buffer
* spi_transmit_byte
* spi_receive_buffer
* spi_receive_byte
* spi_init (the SPI speed must be around 400 kHz to safely initialize the SD card)
* spi_change_to_high_speed (changes SPI speed after initializing the SD card, may be as fast as supported by the SD)
* delay_100us_res
* get_calendar_time (if applicable)

Small note: some cards may need MISO pull-up.

After providing the functions above, declare the interface struct in your program and initialize it using

* initialize_fatfs_interface(interface_struct)

Some extras can be modified through _#defines_ in _ffconf.h_, like turning on/off _RTC_, enabling _exFAT_, etc.

See the examples for actual implementation.
