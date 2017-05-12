# STM32F0xx-firmwares
Three firmwares for STM32F0xx MCUs developed during a series of System-on-chip lectures at Politecnico di Torino. Tested bare-metal on the STM32F0-Discovery board.
Each folder contains:
1. Firmware code (.c, .h files);
2. Binary (.axf file);
3. PDF report.

## Low power modes
Demonstration of the MCUs low-power modes (4 in total). Current consumption was measured in each state and compared with the datasheet values.

## Wave generation
The purpose of this lab sessions was to get familiar with the DMA, ADC and TIM2 to implement a wave generator program. Additionally, the program was integrated with knowledge acquired in the previous lab session, in fact the micro-controller was intermittently forced in a low power state.

## IoT application
This project involves the Silica Branca Wi-Fi module to implement a sample IoT application. 
A user can interact via a webpage with the board. HTML pages are hosted on the Branca board's Flash memory which acts as a server. In addition it receives AT commands from the user and relays them via UART to the STM32F0-Discovery board, which in turn turns on/off an LED based on the received message.

