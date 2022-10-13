/******************************************************************************
 @copyright Copyright © 2022 by Jason Duffy.
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
******************************************************************************/

/**
 * @file rotary_encoder.h
 * @ingroup rotary_encoder
 * @author Jason Duffy
 * @date 30th September 2022
 * @brief Algorithms for incremental encoder decoding, with rejection of
 * invalid signals, for the STM32f4xx HAL. Upto 5 incremental encoders with
 * push button can be used.
 */

#ifndef ROTARY_ENCODER_DOT_H
#define ROTARY_ENCODER_DOT_H

#include <stdbool.h>
#include <stdint.h>
#include "gpio.h"

/**
 * Handle struct to store config, pinout and state for each encoder. 
 * Instatiate for each encoder to be used. 
 */
typedef struct
{
    // Pinout for encoder wiring. Each pin must be linked to an EXTI interrupt.
    uint16_t pin_a;
    uint16_t pin_b;
    uint16_t button_pin;
    GPIO_TypeDef *port_a;
    GPIO_TypeDef *port_b;

    // Counter value, initialised to 0 as default. 
    int16_t counter;

    /*
     * Reset value, 0 as default. When button is pushed, counter will be reset
     * to this value.
     */
    int16_t reset_value;

    // Min and max values for the counter to be confined to.
    int16_t counter_max;
    int16_t counter_min;

    /*
     * These can be ignored when instantiating the struct, as they do not need
     * to be configured. 
     */
    uint8_t old_state;
    uint8_t new_state;
}rot_enc_handle_t;


/**
 * Initialises and registers each encoder. Returns false if failed due to
 * registry array being full (Max No. of encoders exceeded). 
 * Call this function for each encoder, passing each rot_enc_handle_t struct
 * pointer into the init function.
 * @param takes a pointer to a rot_enc_handle_t object. 
 * @return returns true if registry was successful, false if not. 
 */
bool init_rotary_encoder(rot_enc_handle_t *rot_enc_handle_ptr);


/**
 * Insert this function into your overridden definition of
 * HAL_GPIO_EXTI_Callback(). 
 * This function will determine which encoder triggered the interrupt,
 * determine whether the input is valid, and increment/decrement the counter.
 * (Or reset it if button pushed).
 * @param takes the GPIO pin number that triggered the interrupt.
 */
void rot_enc_callback(uint16_t GPIO_Pin);


/**
 * @param takes a pointer to a rot_enc_handle_t object.
 * @return the current counter value for the specified encoder.
 */
int16_t rot_enc_get_count_value(rot_enc_handle_t *rot_enc_handle_ptr);

#endif // ROTARY_ENCODER_DOT_H


// End of file. // 
