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
 * @file rotary_encoder.c
 * @ingroup rotary_encoder
 * @author Jason Duffy
 * @date 30th September 2022
 * @brief Algorithms for incremental encoder decoding, with rejection of
 * invalid signals, for the STM32f4xx HAL. 
 */

#include "rotary_encoder.h"
#include "stm32f4xx_hal.h"

#include "log_system.h"

#define MAX_NUM_OF_ENCODERS   5

// -------- Log system configuration. -------- //
log_system_config_t log_rot_enc = 
{
  .p_system_tag = "Rotary_Encoder",
  .file_log_level = VERBOSE_DEBUG,
};

// ------------------------------------------------------------------------- //
// ------------------------- File scope variables -------------------------- //
// ------------------------------------------------------------------------- //

/**
 * Local storage for device handle pointers.
 */
static rot_enc_handle_t *registered_handles[MAX_NUM_OF_ENCODERS] = {NULL};


/**
 * Lookup table to determine if a transition is valid.
 * 4 bit phase transition value from encoder corresponds to decimal index 0-15.
 */
static int8_t rot_enc_lookup_table[16] =
{0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};


// ------------------------------------------------------------------------- //
// --------------------- Utility function prototypes ----------------------- //
// ------------------------------------------------------------------------- // 
uint8_t get_state(rot_enc_handle_t *handle_ptr);
rot_enc_handle_t* determine_trigger(uint16_t GPIO_Pin);
void decode_phase_transition(rot_enc_handle_t *handle_ptr);
void print_debug_info(rot_enc_handle_t *handle_ptr);


// ------------------------------------------------------------------------- //
// ---------------------- Public function defintions ----------------------- //
// ------------------------------------------------------------------------- //

/*
 * Initialises and registers each encoder. Returns false if failed due to
 * registry array being full (Max No. of encoders exceeded). 
 * Call this function for each encoder, passing each rot_enc_handle_t struct
 * pointer into the init function.
 * @param takes a pointer to a rot_enc_handle_t object. 
 * @return returns true if registry was successful, false if not. 
 */
bool init_rotary_encoder(rot_enc_handle_t* handle_ptr)
{
  bool registration_success = false;

  for (int index = 0; index < MAX_NUM_OF_ENCODERS; ++index)
  {
    if (registered_handles[index] == NULL)
    {
      registered_handles[index] = handle_ptr;
      registration_success = true;
      break;
    }
  }
  //print_debug_info(handle_ptr);
  return registration_success;
}


/*
 * Insert this function into your overridden definition of
 * HAL_GPIO_EXTI_Callback(). 
 * This function will determine which encoder triggered the interrupt,
 * determine whether the input is valid, and increment/decrement the counter.
 * (Or reset it if button pushed).
 * @param takes the GPIO pin number that triggered the interrupt.
 */
void rot_enc_callback(uint16_t GPIO_Pin)
{
  // Local variable to store handle pointer for encoder that triggered ISR. 
  rot_enc_handle_t *handle_ptr = determine_trigger(GPIO_Pin);

  // If button was pushed, reset count.
  if (handle_ptr->button_pin == GPIO_Pin)
  {
    handle_ptr->counter = handle_ptr->reset_value;
  }

  //  If rotary encoder pins triggered interrupt, run encoder algorithm.
  else
  {
    decode_phase_transition(handle_ptr);
  }
}


/**
 * @param takes a pointer to a rot_enc_handle_t object.
 * @return the current counter value for the specified encoder.
 */
int16_t rot_enc_get_count_value(rot_enc_handle_t* handle_ptr)
{
    return handle_ptr->counter;
}


// ------------------------------------------------------------------------- //
// ------------------------- Private Utility Functions --------------------- //
// ------------------------------------------------------------------------- //


/**
 * @param takes a pointer to a rot_enc_handle_t object.
 * @return digital state of pins A and B from inside handle struct, as a 2 bit
 * number.
 */
uint8_t get_state(rot_enc_handle_t *handle_ptr)
{
  uint8_t temp = 0;
  // Pack Pin_A and Pin_B values into a single variable 0b000000AB.
  temp = HAL_GPIO_ReadPin(handle_ptr->port_a, handle_ptr->pin_a) << 1;
  temp |= HAL_GPIO_ReadPin(handle_ptr->port_b, handle_ptr->pin_b);
  return temp; 
}


/**
 * @param takes the GPIO pin number that triggered the interrupt.
 * @return pointer to the handle struct of the encoder that triggered the
 * interrupt.
 */ 
rot_enc_handle_t* determine_trigger(uint16_t GPIO_Pin)
{
  rot_enc_handle_t *handle_ptr = NULL;

  for (int index = 0; index < MAX_NUM_OF_ENCODERS; ++index)
  {
    if (registered_handles[index]->pin_a == GPIO_Pin ||
        registered_handles[index]->pin_b == GPIO_Pin ||
        registered_handles[index]->button_pin == GPIO_Pin)
    {
      handle_ptr = registered_handles[index];
      break;
    }
  }
  return handle_ptr;
}


/**
 * Decodes the phase transition from the encoder, and stores the new state.
 * @param takes a pointer to a rot_enc_handle_t object.
 */
void decode_phase_transition(rot_enc_handle_t *handle_ptr)
{
  // Get new pin states. 
  handle_ptr->new_state = get_state(handle_ptr);

  // Pack new pin states into nibble with old pin state. 
  uint8_t transition = (handle_ptr->old_state << 2) |
                       (handle_ptr->new_state);

  // Use lookup table to edit counter ONLY if pin transitions are valid.
  // (lookup_value = 0 if invalid.)
  int8_t lookup_value = rot_enc_lookup_table[transition];

  // Test if we are decrementing. 
  if (lookup_value < 0)
  {
    // Check lower limit. 
    if (handle_ptr->counter > handle_ptr->counter_min)
    {
      handle_ptr->counter += lookup_value;
    }
  }
  // Test if we are incrementing.
  else if (lookup_value > 0)
  {
    // Check upper limit. 
    if (handle_ptr->counter < handle_ptr->counter_max)
    {
      handle_ptr->counter += lookup_value;
    }
  }

  // Update old state for next run.
  handle_ptr->old_state = handle_ptr->new_state;
}


/**
 * Utility function to print debug info from a given encoder handle struct.
 * @param takes a pointer to a rot_enc_handle_t object.
 */ 
void print_debug_info(rot_enc_handle_t *handle_ptr)
{
    log_message_with_unsigned_val(&log_rot_enc,
                                  DEBUG,
                                  "handle_ptr->pin_a =",
                                  handle_ptr->pin_a,
                                  HEXADECIMAL);

    log_message_with_unsigned_val(&log_rot_enc,
                                  DEBUG,
                                  "handle_ptr->pin_b =",
                                  handle_ptr->pin_b,
                                  HEXADECIMAL);

    log_message_with_unsigned_val(&log_rot_enc,
                                  DEBUG,
                                  "handle_ptr->button_pin =",
                                  handle_ptr->button_pin,
                                  HEXADECIMAL);

    log_message_with_unsigned_val(&log_rot_enc,
                                  DEBUG,
                                  "handle_ptr->port_a* =",
                                  (uint32_t)handle_ptr->port_a,
                                  HEXADECIMAL);

    log_message_with_unsigned_val(&log_rot_enc,
                                  DEBUG,
                                  "handle_ptr->port_b* =",
                                  (uint32_t)handle_ptr->port_b,
                                  HEXADECIMAL);

    log_message_with_signed_val(&log_rot_enc,
                                  DEBUG,
                                  "handle_ptr->counter =",
                                  handle_ptr->counter,
                                  DECIMAL);

    log_message_with_signed_val(&log_rot_enc,
                                  DEBUG,
                                  "handle_ptr->counter_max =",
                                  handle_ptr->counter_max,
                                  DECIMAL);

    log_message_with_signed_val(&log_rot_enc,
                                  DEBUG,
                                  "handle_ptr->counter_min =",
                                  handle_ptr->counter_min,
                                  DECIMAL);

    log_message_with_unsigned_val(&log_rot_enc,
                                  DEBUG,
                                  "handle_ptr->old_state =",
                                  handle_ptr->old_state,
                                  DECIMAL);

    log_message_with_unsigned_val(&log_rot_enc,
                                  DEBUG,
                                  "handle_ptr->new_state =",
                                  handle_ptr->new_state,
                                  DECIMAL);
}

// End of file. // 
