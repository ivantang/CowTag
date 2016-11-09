/*
 * Copyright (c) 2015-2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __BOARD_H
#define __BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ti/drivers/Power.h>

#include "CC1310_LAUNCHXL.h"

/* These #defines allow us to reuse TI-RTOS across other device families */
#define     Board_LED0              Board_RLED
#define     Board_LED1              Board_GLED
#define     Board_LED2              Board_LED0

#define     Board_ADC0              CC1310_LAUNCHXL_ADCVSS
#define     Board_ADC1              CC1310_LAUNCHXL_ADCVDDS

#define     Board_ADCBuf0           CC1310_LAUNCHXL_ADCBuf0
#define     Board_ADCBufChannel0    (0)
#define     Board_ADCBufChannel1    (1)

#define     Board_BUTTON0           Board_BTN1
#define     Board_BUTTON1           Board_BTN2

#define     Board_UART0             Board_UART
#define     Board_AES0              Board_AES
#define     Board_WATCHDOG0         CC1310_LAUNCHXL_WATCHDOG0

#define		Board_I2C0				Board_I2C
#define		Board_I2C_ACCEL			Board_I2C0

#define     Board_initGeneral() { \
    Power_init(); \
    if (PIN_init(BoardGpioInitTable) != PIN_SUCCESS) \
        {System_abort("Error with PIN_init\n"); \
    } \
}

#define     Board_initGPIO()
#define     Board_initPWM()        PWM_init()
#define		Board_initI2C()			I2C_init()
#define     Board_initSPI()         SPI_init()
#define     Board_initUART()        UART_init()
#define     Board_initWatchdog()    Watchdog_init()
#define     Board_initADCBuf()      ADCBuf_init()
#define     Board_initADC()         ADC_init()
#define     GPIO_toggle(n)
#define     GPIO_write(n,m)

/* Board specific I2C addresses */

//#define     Board_HDC1000_ADDR      (0x43)
/*LIS3DH ACCELEROMETER ADDRESSES*/
#define     Board_LIS3DH_ADDR		 0x18 // 0011001
#define 	LIS3DH_REG_STATUS1       0x07
#define 	LIS3DH_REG_OUTADC1_L     0x08
#define 	LIS3DH_REG_OUTADC1_H     0x09
#define		LIS3DH_REG_OUTADC2_L     0x0A
#define 	LIS3DH_REG_OUTADC2_H     0x0B
#define		LIS3DH_REG_OUTADC3_L     0x0C
#define 	LIS3DH_REG_OUTADC3_H     0x0D
#define		LIS3DH_REG_INTCOUNT      0x0E
#define 	LIS3DH_REG_WHOAMI        0x0F
#define 	LIS3DH_REG_TEMPCFG       0x1F
#define 	LIS3DH_REG_CTRL1         0x20
#define 	LIS3DH_REG_CTRL2         0x21
#define 	LIS3DH_REG_CTRL3         0x22
#define 	LIS3DH_REG_CTRL4         0x23
#define 	LIS3DH_REG_CTRL5         0x24
#define 	LIS3DH_REG_CTRL6         0x25
#define 	LIS3DH_REG_REFERENCE     0x26
#define 	LIS3DH_REG_STATUS2       0x27
#define 	LIS3DH_REG_OUT_X_L       0x28
#define 	LIS3DH_REG_OUT_X_H       0x29
#define 	LIS3DH_REG_OUT_Y_L       0x2A
#define 	LIS3DH_REG_OUT_Y_H       0x2B
#define 	LIS3DH_REG_OUT_Z_L       0x2C
#define 	LIS3DH_REG_OUT_Z_H       0x2D
#define 	LIS3DH_REG_FIFOCTRL      0x2E
#define 	LIS3DH_REG_FIFOSRC       0x2F
#define 	LIS3DH_REG_INT1CFG       0x30
#define 	LIS3DH_REG_INT1SRC       0x31
#define 	LIS3DH_REG_INT1THS       0x32
#define 	LIS3DH_REG_INT1DUR       0x33
#define 	LIS3DH_REG_CLICKCFG      0x38
#define 	LIS3DH_REG_CLICKSRC      0x39
#define 	LIS3DH_REG_CLICKTHS      0x3A
#define 	LIS3DH_REG_TIMELIMIT     0x3B
#define 	LIS3DH_REG_TIMELATENCY   0x3C
#define 	LIS3DH_REG_TIMEWINDOW    0x3D
//#define     Board_OPT3001_ADDR      (0x45)
//#define     Board_BMP280_ADDR       (0x77)


/* Interface #1 */
//#define     Board_MPU9250_ADDR      (0x68)
//#define     Board_MPU9250_MAG_ADDR  (0x0C)


#ifdef __cplusplus
}
#endif

#endif /* __BOARD_H */
