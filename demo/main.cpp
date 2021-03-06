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

#ifdef _cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ch.h"
#include "hal.h"

#include "shell.h"
#include "chprintf.h"


static const I2CConfig i2cconfig = {
    OPMODE_I2C,
    400000,
    FAST_DUTY_CYCLE_2,
};

#ifdef _cplusplus
}
#endif

#include <cstdlib>

#include "shellblink.hpp" //
#include <PCA9685.hpp>


PCA9685 *pcachip;

//PCA9685 pcachip;

void cmd_reg(BaseSequentialStream *chp, int argc, char *argv[]) {

  (void)argv;
  if (argc > 1) {
    chprintf(chp, "Usage: reg [regnum]\r\n");
    return;
  }

  if (argc == 0)
  {
    chprintf(chp, "MODE1 = %u \r\n", pcachip->getRegisterValue(PCA9685_MODE1));
    chprintf(chp, "PRESCALE = %u \r\n", pcachip->getRegisterValue(PCA9685_PRESCALE));
    chprintf(chp, "STATUS = %u \r\n", pcachip->getStatus());
  }

  if (argc == 1)
  {
    uint8_t reg = atoi(argv[0]);
    chprintf(chp, "REGISTER[%u] = %u \r\n", reg, pcachip->getRegisterValue(reg));
  }
}

void cmd_freq(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)argv;
  if (argc != 1) {
    chprintf(chp, "Usage: freq <freq>\r\n");
    return;
  }

  uint16_t freq = atoi(argv[0]);
  chprintf(chp, "FREQ : %u \r\n", pcachip->setFreq(freq));
  chprintf(chp, "STATUS : %u \r\n", pcachip->getStatus());
}

void cmd_pwm(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)argv;
  if (argc != 3) {
    chprintf(chp, "Usage: pwm <channel> <on> <off>\r\n");
    return;
  }

  uint8_t channel = atoi(argv[0]);
  uint16_t on = atoi(argv[1]);
  uint16_t off = atoi(argv[2]);
  pcachip->setPWM(channel, on, off);
  chprintf(chp, "STATUS : %u \r\n", pcachip->getStatus());
}


/*
 * assert Shell Commands to functions
 */
static const ShellCommand commands[] = {
  {"mem", cmd_mem},
  {"threads", cmd_threads},
  {"reg", cmd_reg},
  {"freq", cmd_freq},
  {"pwm", cmd_pwm},
  {NULL, NULL}
};



/*
 * Shell configuration
 */

#define SHELL_WA_SIZE   THD_WA_SIZE(2048)

static const ShellConfig shell_cfg1 = {
  (BaseSequentialStream *)&SD2,
  commands
};



/*
 * Application entry point.
 */
int main(void) {
  /*
   * Shell thread
   */
  Thread *shelltp = NULL;

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  /*
   * Activates the serial driver 2 using the driver default configuration.
   * PA2(TX) and PA3(RX) are routed to USART2.
   */
  sdStart(&SD2, NULL);
  palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7));
  palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7));


  pcachip = new PCA9685(&PCA9685_DEFI2C_DRIVER, &i2cconfig, PCA9685_ADDRESS, PCA9685_FREQ, true);
  pcachip->setPWM(0, 150, 600);
  chThdSleepMilliseconds(2000);
  pcachip->setPWM(0, 0, 150);


  //pcachip.setPWM(0,200, 400);

  startBlinker();

  /*
   * Main loop, does nothing except spawn a shell when the old one was terminated
   */
  while (TRUE) {
    if (!shelltp)
      {
        shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
      }
    else if (chThdTerminated(shelltp)) {
      chThdRelease(shelltp);    /* Recovers memory of the previous shell.   */
      shelltp = NULL;           /* Triggers spawning of a new shell.        */
    }
    chThdSleepMilliseconds(1000);
  }
}
