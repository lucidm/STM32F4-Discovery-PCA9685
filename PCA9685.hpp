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
#ifndef _PCA9685_HPP_
#define _PCA9685_HPP_

/*
 * According to:
 * http://www.nxp.com/documents/data_sheet/PCA9685.pdf
 */

/*
 * Internal clock frequency - 25Mhz
 */
#define PCA9685_CLOCK 25000000

#define PCA9685_ADDRESS 0x40
/*
 * Default PWM frequency.
 */
#define PCA9685_FREQ 60

/*
 * Using I2CD2
 * I2C2_SDA PB10
 * I2C2_SCL PB11
 *
 */
static const I2CConfig PCA9685_I2C_CONFIG = {
    OPMODE_I2C,
    400000,
    FAST_DUTY_CYCLE_2,
};

/* See 7.3 Register definitions */
#define PCA9685_MODE1 0x00
#define PCA9685_MODE2 0x01
#define PCA9685_ALLCALLADR 0x05
#define PCA9685_PRESCALE 0xFE

/* See 7.3.1 Mode register 1, MODE1 */
#define PCA9685_MODE1_RESTART 0x80
#define PCA9685_MODE1_EXTCLK 0x40
#define PCA9685_MODE1_AI 0x20
#define PCA9685_MODE1_SLEEP 0x10
#define PCA9685_MODE1_SUB1 0x08
#define PCA9685_MODE1_SUB2 0x04
#define PCA9685_MODE1_SUB3 0x02
#define PCA9685_MODE1_ALLCALL 0x01

/* 7.3.2 Mode register 2, MODE2 */
#define PCA9685_MODE2_INVRT 0x10
#define PCA9685_MODE2_OCH 0x08
#define PCA9685_MODE2_OUTDRV 0x04
#define PCA9685_MODE2_OUTNE 0x03 //Actually a mask of bits. Paragraph 7.3.2 Mode register 2, MODE2

#define LED0_ON_L 0x6
#define LED0_ON_H 0x7
#define LED0_OFF_L 0x8
#define LED0_OFF_H 0x9

#define PCA9685_DEFI2C_PORT GPIOB
#define PCA9685_DEFI2C_SCL_PAD 10
#define PCA9685_DEFI2C_SDA_PAD 11
#define PCA9685_DEFI2C_MODE PAL_STM32_OTYPE_OPENDRAIN | PAL_STM32_OSPEED_MID2 | PAL_MODE_ALTERNATE(4)
#define PCA9685_DEFI2C_SPEED 400000
#define PCA9685_DEFI2C_DRIVER I2CD2

class PCA9685 {
    public:
        PCA9685(I2CDriver *driver, const I2CConfig *config, uint8_t address = PCA9685_ADDRESS, uint16_t freq = PCA9685_FREQ, uint8_t acquirebus = false);
        PCA9685();
        ~PCA9685();

        void reset(void);
        uint8_t setAddress(uint8_t address);
        uint16_t setFreq(uint16_t freq);
        void setPWM(uint8_t channel, uint16_t on, uint16_t off);
        void setPWM(uint8_t channel, const uint16_t *on, const uint16_t *off, uint8_t count);
        void setPeriod(uint8_t channel, float period, uint8_t duty);
        uint32_t getPWM(uint8_t channel);
        uint16_t getRegisterValue(uint8_t reg);
        void acquireBus(uint8_t exclusive);
        i2cflags_t getStatus();
        uint8_t getAddress();



    private:
        uint8_t i2caddres;
        I2CDriver *driver;
        const I2CConfig *config;
        uint8_t acquire;

        uint8_t rxbuff[32];
        uint8_t txbuff[32];
        uint16_t freq;
        msg_t status;

        void writereg(uint8_t reg, uint8_t data);
        uint8_t readreg(uint8_t reg);

};

#endif /* _PCA9685_HPP_ */