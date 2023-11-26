# Raw GPS IQ logger based on the RP2040
This repository contains the firmware for a Raspberry Pi RP2040-based GPS logging device. The device uses a MAX2769 universal GPS receiver IC to perform the required filtering and sampling of the IF representation of the signal. The RP2040 then reads the data using one of its PIO engines and writes it to an SD card using the other PIO engine emulating an SDIO peripheral.

# Setup
This project uses the Raspberry Pi Pico C/C++ SDK, and the structure is largely based on the instructions at https://www.raspberrypi.com/documentation/microcontrollers/c_sdk.html. Note that on Windows you will need to either use the Pico SDK setup script, or manually install the dependencies listed in Section 9.2 of https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf.
