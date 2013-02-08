STM32F4 Discovery library with driver written as C++ class, for accessing NXA PCA9685 (16 channel PWM I2C driver) chip under ChibiOS environment.

requirements
------------
* Chibios 2.5.0+ (or get a recent development snapshot)
* arm toolchain


usage
-----
* cd ./demo
* edit the Makefile and point "CHIBIOS = ../../ChibiOS" to your ChibiOS folder
* connect the STM32F4 Discovery CN1 USB port with your desktop USB port
* connect PA2/PA3 as RxD and TxD respectively to your serial dongle, plug it to USB desktop port
  (alternatively you can change code to use CN5 USB port on STM32F4 Discovery as serial port and change shell config)
* make
* flash the STM32F4: st-flash write build/ch.bin 0x8000000
* use your favorite terminal programm to connect to the Serial Port (ie. /dev/ttyACM0, or COM4 on Windows) at 38400 bps
* after flashing, blue led on the board should start to blink

shell commands
--------------
mem - memory usage
threads - current thread list with statuses
reg [number] - returns value of given register. Register number is optional, if not given returns values of MODE1,
               PRESCALE and last I2C transfer status
freq <hz>  - set PWM frequency in Hz
pwm <channel> <oncnt> <offcnt> - set PWM values for given channel with values for "on" and "off" cycles
per <channel> <period> <duty> - set PWM value for given channel with given (floating point) period and duty value



links
-----
ChibiOS homepage - http://www.chibios.org/
Datasheet for PCA9685 - http://www.nxp.com/documents/data_sheet/PCA9685.pdf
STM32F4 Discovery board - http://www.st.com/internet/evalboard/product/252419.jsp
Ready to use board with the chip - http://www.adafruit.com/products/815

By default, driver uses I2CD2 port, if you change port to the other one, please change pal modes for proper GPIO pins in
PCA9685 constructor.
Please read comments in PCA9685.cpp and PCA9685.hpp.