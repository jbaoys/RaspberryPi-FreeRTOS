/*
    FreeRTOS V7.2.0 - Copyright (C) 2012 Real Time Engineers Ltd.


    ***************************************************************************
     *                                                                       *
     *    FreeRTOS tutorial books are available in pdf and paperback.        *
     *    Complete, revised, and edited pdf reference manuals are also       *
     *    available.                                                         *
     *                                                                       *
     *    Purchasing FreeRTOS documentation will not only help you, by       *
     *    ensuring you get running as quickly as possible and with an        *
     *    in-depth knowledge of how to use FreeRTOS, it will also help       *
     *    the FreeRTOS project to continue with its mission of providing     *
     *    professional grade, cross platform, de facto standard solutions    *
     *    for microcontrollers - completely free of charge!                  *
     *                                                                       *
     *    >>> See http://www.FreeRTOS.org/Documentation for details. <<<     *
     *                                                                       *
     *    Thank you for using FreeRTOS, and thank you for your support!      *
     *                                                                       *
    ***************************************************************************


    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    >>>NOTE<<< The modification to the GPL is included to allow you to
    distribute a combined work that includes FreeRTOS without being obliged to
    provide the source code for proprietary components outside of the FreeRTOS
    kernel.  FreeRTOS is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details. You should have received a copy of the GNU General Public
    License and the FreeRTOS license exception along with FreeRTOS; if not it
    can be viewed here: http://www.freertos.org/a00114.html and also obtained
    by writing to Richard Barry, contact details for whom are available on the
    FreeRTOS WEB site.

    1 tab == 4 spaces!

    ***************************************************************************
     *                                                                       *
     *    Having a problem?  Start by reading the FAQ "My application does   *
     *    not run, what could be wrong?                                      *
     *                                                                       *
     *    http://www.FreeRTOS.org/FAQHelp.html                               *
     *                                                                       *
    ***************************************************************************


    http://www.FreeRTOS.org - Documentation, training, latest information,
    license and contact details.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool.

    Real Time Engineers ltd license FreeRTOS to High Integrity Systems, who sell
    the code with commercial support, indemnification, and middleware, under
    the OpenRTOS brand: http://www.OpenRTOS.com.  High Integrity Systems also
    provide a safety engineered and independently SIL3 certified version under
    the SafeRTOS brand: http://www.SafeRTOS.com.
*/


#include <FreeRTOS.h>
#include <task.h>
#include <stdbool.h>
#include <semphr.h>

#include "Drivers/irq.h"
#include "Drivers/gpio.h"

#define GPIO_PIN_16 16
#define GPIO_PIN_12 12
#define GPIO_PIN_21 21
#define LED_BLINK_DELAY 100
#define LED_BLINK_DELAY_HALF (LED_BLINK_DELAY/2)

static int siren_length = 0;
static bool RED_LED = false;
xSemaphoreHandle xSemaphore = NULL;

void task1(void *pParam) {
    bool ON = false;
    while(1) {
        if ( xSemaphore != NULL &&
             (xSemaphoreTake( xSemaphore, ( portTickType ) 10 ) == pdTRUE ) )
        {
            ON = siren_length && RED_LED;
            RED_LED = !RED_LED;
            if (siren_length) {
                --siren_length;
            }
            xSemaphoreGive( xSemaphore );
        }
        SetGpio(GPIO_PIN_16, ON);
        vTaskDelay(LED_BLINK_DELAY);
    }
}

void task2(void *pParam) {
    bool ON = false;
    while(1) {
        if ( xSemaphore != NULL &&
             (xSemaphoreTake( xSemaphore, ( portTickType ) 10 ) == pdTRUE ) )
        {
            ON = siren_length && RED_LED;
            xSemaphoreGive( xSemaphore );
        }
        SetGpio(GPIO_PIN_12, ON);
        vTaskDelay(LED_BLINK_DELAY);
    }
}

void task3(void *pParam) {
    int siren_state=1, a, ct=0;
    bool debounce = false;
    // Create the semaphore to guard a shared resource.
    vSemaphoreCreateBinary( xSemaphore );

    while(1) {
        if (ct) {
            ct--;
            taskYIELD();
        } else {
            a = ReadGpio(GPIO_PIN_21);
            if (siren_state != a) {
                if (debounce) {
                    debounce = false;
                    // confirm that siren_state is changed
                    siren_state = a;
                    if (siren_state == 0) {
                        // 0 means it is triggered, and set the siren_length to a peirod of time for alarming.
                        if ( xSemaphore != NULL &&
                             (xSemaphoreTake( xSemaphore, ( portTickType ) 10 ) == pdTRUE ) )
                        {
                            siren_length = 120;
                            xSemaphoreGive( xSemaphore );
                        }
                    }
                } else {
                    // debounce
                    debounce = true;
                    ct = 100;
                }
            } else {
                debounce = false;
            }
        }
    }
}

/**
 *  This is the systems main entry, some call it a boot thread.
 *
 *  -- Absolutely nothing wrong with this being called main(), just it doesn't have
 *  -- the same prototype as you'd see in a linux program.
 **/
void main (void)
{
    SetGpioFunction(GPIO_PIN_16, 1);         // RDY led
    SetGpioFunction(GPIO_PIN_12, 1);
    SetGpioFunction(GPIO_PIN_21, 0);

    xTaskCreate(task1, "LED_0", 128, NULL, 0, NULL);
    xTaskCreate(task2, "LED_1", 128, NULL, 0, NULL);
    xTaskCreate(task3, "Trigger", 128, NULL, 0, NULL);

    vTaskStartScheduler();

    /*
     *  We should never get here, but just in case something goes wrong,
     *  we'll place the CPU into a safe loop.
     */
    while(1) {
        ;
    }
}
