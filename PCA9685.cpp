/*
    ChibiOS/PCA9685 - Copyright (C) 2013
                 Jarek Zok <jarek.zok@fwioo.pl>

    This file is part of ChibiOS/PCA9685.

    ChibiOS/PCA9685 is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS/PCA9685 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>

#ifdef _cplusplus
extern "C" {
#endif
#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#ifdef _cplusplus
}
#endif

#include <cmath>
#include <cstdlib>

#include "PCA9685.hpp"


/**
 * PCA9685 class implements methods to access the chip.
 *
 * @param I2CDriver *driver I2C driver used to communicate with the chip
 * @param const I2CConfig *config I2C configuration structure
 * @param uint8_t address Address of the chip, default space starts at 0x40
 * @param uint16_t freq Default frequency of PWM
 *
 */
PCA9685::PCA9685(I2CDriver *driver, const I2CConfig *config, uint8_t address, uint16_t freq) {
    this->i2caddres = address;
    this->driver = driver;
    this->freq = freq;
    this->config = config;

     i2cStart(this->driver, this->config);
     palSetPadMode(GPIOB, 10, PAL_STM32_OTYPE_OPENDRAIN | PAL_STM32_OSPEED_MID2 | PAL_MODE_ALTERNATE(4));
     palSetPadMode(GPIOB, 11, PAL_STM32_OTYPE_OPENDRAIN | PAL_STM32_OSPEED_MID2 | PAL_MODE_ALTERNATE(4));

     this->reset();
     this->setFreq(this->freq);
}

/**
 * Static constructor of the class. Some of the parameters are defined in header.
 * PCA9685_ADDRESS address of the chip (0x40).
 * PCA9685_DEFI2C_DRIVER as I2C driver (I2CD2 by default)
 * PCA9685_FREQ as default frequency used
 * PCA9685_I2C_CONFIG is static structure with proper I2C parameters set.
 */
PCA9685::PCA9685() {
    this->i2caddres = PCA9685_ADDRESS;
    this->driver = &PCA9685_DEFI2C_DRIVER;
    this->freq = PCA9685_FREQ;
    this->config = &PCA9685_I2C_CONFIG;

     i2cStart(this->driver, this->config);
     palSetPadMode(GPIOB, 10, PAL_STM32_OTYPE_OPENDRAIN | PAL_STM32_OSPEED_MID2 | PAL_MODE_ALTERNATE(4));
     palSetPadMode(GPIOB, 11, PAL_STM32_OTYPE_OPENDRAIN | PAL_STM32_OSPEED_MID2 | PAL_MODE_ALTERNATE(4));

     this->reset();
     this->setFreq(this->freq);
}

/**
 * Releases bus and device
 *
 *
 */
PCA9685::~PCA9685() {
    i2cStop(this->driver);
}

/**
 * Send reset to chip, wait for oscillator to stabilize.
 *
 */
void PCA9685::reset(void) {
    this->writereg(PCA9685_MODE1, 0x00);
    chThdSleepMilliseconds(1);//Wait for oscillator stabilisation
}

/**
 * Set PWM frequency in Hz.
 * @param uint16_t freq - 40Hz min. to 10KHz max, if external 50MHz clock for the chip is provided
 *
 * @return uint16_t previous frequency value
 *
 */
uint16_t PCA9685::setFreq(uint16_t freq) {

    uint16_t oldfreq = this->freq;

    //According to NXP documentation (7.3.5 PWM frequency PRE_SCALE):
    // prescale = round((osc_clock / (4096 x rate)) - 1)
    // where osc_clock - default oscillator clock set to 25Mhz, requested rate is in Hz
    uint8_t prescale = (uint8_t) floor((PCA9685_CLOCK / (4096 * freq)) - 1);
    if (prescale < 3) //Shouldn't be less than 3
        prescale = 3;

    uint8_t oldmode = this->readreg(PCA9685_MODE1); //Preserve old mode
    uint8_t newmode = (oldmode & !PCA9685_MODE1_RESTART) | PCA9685_MODE1_SLEEP; // Don't change any MODE1 bit except set SLEEP bit 1 and RESTART bit goes 0
    this->writereg(PCA9685_MODE1, newmode); // go to sleep, shutting the oscillator off
    this->writereg(PCA9685_PRESCALE, prescale); // set the prescaler
    this->writereg(PCA9685_MODE1, oldmode);
    chThdSleepMicroseconds(500);//Wait 0.5ms for oscillator stabilisation
    this->writereg(PCA9685_MODE1, oldmode | PCA9685_MODE1_ALLCALL | PCA9685_MODE1_AI | PCA9685_MODE1_RESTART);
    this->freq = freq;

    return oldfreq;
}

/**
 * Set PWM period for given channel.
 * @param uint8_t channel [0-15] Channel number. 0 to 15
 * @param uint16_t on - internal chip counter should count to this value, to turn channel on
 * @param uint16_t off - internal chip counter should count to this value, to turn channel off
 *
 * Both on and off parameters don't say anything about real period which depend on current frequency.
 * Thanks to, you can overlap on and off values or even set different phase shift on each
 * channel independently.
 */
void PCA9685::setPWM(uint8_t channel, uint16_t on, uint16_t off) {

  this->txbuff[0] = LED0_ON_L+4*channel;
  this->txbuff[1] = on & 0x00FF;
  this->txbuff[2] = on >> 8;
  this->txbuff[3] = off & 0x00FF;
  this->txbuff[4] = off >> 8;

  this->status = i2cMasterTransmitTimeout(this->driver, this->i2caddres, this->txbuff, 5, this->rxbuff, 0, 100);
}

/**
 * Set PWM for more than one channel.
 * @param uint8_t channel [0-15] Starting channel number.
 * @param const uint16_t *on Array of values of "on" for subsequent channel
 * @param const uint16_t *off Array of values of "off" for subsequent channel
 * @param uint8_t count Number of channels to set.
 *
 * You can use this function to setup PWM for more than one channel at once.
 * The function will set auto increment bit in MODE1. If sum of counter and channel
 * is greater than 15, it will overwrite PWM of overlapping channels.
 *
 */
void PCA9685::setPWM(uint8_t channel, const uint16_t *on, const uint16_t *off, uint8_t count) {
    uint16_t txbuff[33];

    if (count>15)
        count = 15;
    uint8_t j=0;
    txbuff[0] = LED0_ON_L+4*channel;
    for (uint8_t i=1; i<count; i+=2) {
        txbuff[i] = on[j];
        txbuff[i+1] = off[j+1];
        j++;
    }

    uint8_t oldmode = this->readreg(PCA9685_MODE1); //Preserve old mode
    this->writereg(PCA9685_MODE1, oldmode | PCA9685_MODE1_AI); //Set auto increment
    this->status = i2cMasterTransmitTimeout(this->driver, this->i2caddres, (uint8_t*) txbuff, (count+1) * sizeof(uint8_t), this->rxbuff, 0, 100);
    this->writereg(PCA9685_MODE1, oldmode); //Restore old mode
}

/**
 * Set PWM for given channel using fraction of a second for counter to reach maximum value and given duty cycle in percents.
 * so you can change frequency without duty modification. Of course all channels of PCA9685 use clock source for their
 * counter changing period parameter will influence all channels.
 *
 * @param uint8_t channel - channel number
 * @param float period - fraction of a second for a counter to reach maximum value (in this time counter will count from 0 to 4095)
 * @param uint8_t duty - duty in percent of given period
 */
void PCA9685::setPeriod(uint8_t channel, float period, uint8_t duty) {
    float freq = (100.0 / duty) / period;

    this->setFreq(ceil(freq));
    this->setPWM(channel, 0, (4096/100) * duty);
}

/**
 * Returns PWM value for given channel
 * @param uint8_t channel
 *
 * @return uint32_t upper half of it is set to OFF register value for given channel, lower half is set for ON register value
 *
 */
uint32_t PCA9685::getPWM(uint8_t channel) {
    uint32_t pwm = 0;

    pwm = (this->readreg((LED0_ON_L+4*channel) + 2) & 0x0F) << 8;
    pwm |= this->readreg((LED0_ON_L+4*channel) + 3);
    pwm = pwm << 16; //Shift all by 16 bits, first half contains on period counter
    pwm |= (this->readreg((LED0_ON_L+4*channel) + 1) & 0x0F) << 8;
    pwm |= this->readreg(LED0_ON_L+4*channel); //The other half will contain off period counter

    return pwm;
}

/*
 * Write byte to register
 */
void PCA9685::writereg(uint8_t reg, uint8_t data) {
    this->txbuff[0] = reg;
    this->txbuff[1] = data;

    this->status = i2cMasterTransmitTimeout(this->driver, this->i2caddres, this->txbuff, 2, this->rxbuff, 0, 100);
}

/*
 * Read data from chip register
 */
uint8_t PCA9685::readreg(uint8_t reg) {
    if (reg < 70 || reg > 249)
    {
        this->status = i2cMasterTransmitTimeout(this->driver, this->i2caddres, &reg, 1, this->rxbuff, 1, 100);
    }
    return this->rxbuff[0];
}

/**
 * Returns given register value. If register number is one of LEDx_ON_L or LEDx_OFF_L it will return value of
 * L byte as first and H byte as second one.
 * @param uint8_t reg - register number
 * @return uint16_t - register value
 */
uint16_t PCA9685::getRegisterValue(uint8_t reg)
{
    uint16_t res = 0;
    //Led on/off registers are four 8 bit registers for Low and High bits
    if ((reg >=6 && reg <= 68) && ((reg-6) % 2)==0)
    {
        res = (this->readreg(reg + 1) & 0x0F) << 8;
        res |= this->readreg(reg);
        return res;
    }
    res = this->readreg(reg);
    return res;

}

/**
 * Status of last I2C operation
 * @return msg_t - status
 */
msg_t PCA9685::getStatus() {
    return this->status;
}

/**
 * Change chip address. You can change I2C addres.
 * @param uint8_t address
 */
uint8_t PCA9685::setAddress(uint8_t address) {
    uint8_t t;
    t = this->i2caddres;
    this->i2caddres = address;
    return t;
}

/**
 * Returns current address of the PCA9685
 * @return uint8_t address
 */
uint8_t PCA9685::getAddress() {
    return this->i2caddres;
}