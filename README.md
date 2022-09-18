# STM32_Utilities
A collection of drivers and utilities for the STM32 family of MCU's.

This driver was written for the STM32f4xx family, but you should be able to use it on other STM32 models if you change the HAL header file included in log_system.h.

You'll need to create your project using STM32CUBEMX, with preferences set to generate seperate .h and .c files for each peripheral. You'll also need to set up one of the USART channels under "Connectivity".

It is possible to use the JTAG debugger to interface with the serial lines via your PC, but that requires the CUBEIDE which I'm not too keen on using. 

Instead, I used a serial to USB converter connected to the RX and TX pins, along with the terminal emulator software Coolterm. You can configure Coolterm to print timestamps on each line which can be useful.