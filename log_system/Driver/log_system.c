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
 * @file log_system.c
 * @ingroup log_system
 * @author Jason Duffy
 * @date 18th September 2022
 * @brief Driver file providing logging functionality over UART, to print
 * debug messages and values to a teminal program on your PC. 
 * @bug No known bugs. 
 */

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "log_system.h"
#include "usart.h"

#define TIMEOUT_MS                  100
#define DIGITS_IN_DEC_32_T      10
#define DIGITS_IN_HEX_32_T      8
#define DIGITS_IN_BIN_32_T      32



UART_HandleTypeDef *p_uart_global;

/**
 * Instantiation of log system config object, pass it's address into logging
 * functions. 
 */
log_system_config_t log_system_log =
{
    .p_system_tag = "Log_System",
    .file_log_level = INFO,
};


/**
 * Flag to determine whether logging output is turned on or off globally. The
 * flag itself however is at file scope only. 
 */
static bool log_system_enabled = false;


/**
 * Variable to store the preferred maximum level of logging with global effect.
 * Maximum level available by default. Call log_set_global_max_output_level()
 * to change this value.  
 */
static log_type_t global_max_output_level = VERBOSE_DEBUG;


// Forward declaration - private helper functions. 
static void print_tag_and_log_level(const char *p_tag, log_type_t level);
static void print_unsigned_value_with_format(uint32_t val,
                                            format_type_t format);
static void print_signed_value_with_format(int32_t val, format_type_t format);
static bool log_message_preference_check(log_system_config_t *p_config,
                                            log_type_t level);


/*
 * Initialisation routine - call this function once at startup before using
 * other functions. Log system will then be turned on by default - call
 * log_global_off() if you do not wish it to be. UART must be initialised via
 * the HAL before the log_system may be used. 
 */
void init_log_system(UART_HandleTypeDef *p_uart)
{
    p_uart_global = p_uart;
    log_global_on();
    log_message(&log_system_log, INFO, "Log system initialised");
};


/*
 * Sends only the system tag, log level and message string.
 * @param p_config is a pointer to the log_system config object. Instantiate
 * the config object at the head of each file where logging is required and
 * pass it's address into this function.
 * @param level is the level status of the log message - see log_type_t for
 * available options.
 * @param msg is the message to be logged, enclose it in "" quotation marks.
 */
void log_message(log_system_config_t *p_config,
                 log_type_t level,
                 const char *msg)
{
    // If all test expressions evaluate true, log message.
    if (log_message_preference_check(p_config, level))
    {
        print_tag_and_log_level(p_config->p_system_tag, level);
        HAL_UART_Transmit(p_uart_global, (const uint8_t*)msg,
                        strlen(msg), TIMEOUT_MS);
    }
}  


/*
 * Sends a string, followed by a 32 bit unsigned value (though smaller types
 * such as uint8_t will work fine, they will just be coerced into the 64 bit 
 * variable).
 * @param p_config is a pointer to the log_system config object. Instantiate
 * the config object at the head of each file where logging is required and
 * pass it's address into this function.
 * @param level is the level status of the log message - see log_type_t for
 * available options.
 * @param msg is the message to be logged, enclose it in "" quotation marks.
 * @param val is the numerical value to be logged.
 * @param format is the desired output format of val - see format_type_t for
 * avilable options. 
 */
void log_message_with_unsigned_val(log_system_config_t *p_config,
                                            log_type_t level,
                                            const char *msg,
                                            uint32_t val,
                                            format_type_t format)
{
    if (log_message_preference_check(p_config, level))
    {
        const char space[2] = " \0";
        // Print tag and log level of message. 
        print_tag_and_log_level(p_config->p_system_tag, level);

        // Print message, followed by a space. 
        HAL_UART_Transmit(p_uart_global, (const uint8_t*)msg,
                        strlen(msg), TIMEOUT_MS);
        HAL_UART_Transmit(p_uart_global, (const uint8_t*)space,
                        strlen(space), TIMEOUT_MS);

        // Print numerical value in specified format. 
        print_unsigned_value_with_format(val, format);
    }
}


/*
 * Sends a string, followed by a 32 bit signed value (though smaller types
 * such as uint8_t will work fine, they will just be coerced into the 64 bit 
 * variable).
 * @param p_config is a pointer to the log_system config object. Instantiate
 * the config object at the head of each file where logging is required and
 * pass it's address into this function.
 * @param level is the level status of the log message - see log_type_t for
 * available options.
 * @param msg is the message to be logged, enclose it in "" quotation marks.
 * @param val is the numerical value to be logged.
 * @param format is the desired output format of val - see format_type_t for
 * avilable options. 
 */
void log_message_with_signed_val(log_system_config_t *p_config,
                                            log_type_t level,
                                            const char *msg,
                                            int32_t val,
                                            format_type_t format)
{
    if (log_message_preference_check(p_config, level))
    {
        const char space[2] = " \0";
        // Print tag and log level of message. 
        print_tag_and_log_level(p_config->p_system_tag, level);

        // Print message, followed by a space. 
        HAL_UART_Transmit(p_uart_global, (const uint8_t*)msg,
                        strlen(msg), TIMEOUT_MS);
        HAL_UART_Transmit(p_uart_global, (const uint8_t*)space,
                        strlen(space), TIMEOUT_MS);
        
        // Print numerical value in specified format. 
        print_signed_value_with_format(val, format);
    }
}


/*
 * Sets maximum output level of logging required, to be used at file scope.
 * @param p_config is a pointer to the log_system config object. Instantiate
 * the config object at the head of each file where logging is required and
 * pass it's address into this function.
 * @param level is the maximum level required - see log_type_t for available
 * options.
 */
void log_set_file_max_output_level(log_system_config_t *p_config,
                                   log_type_t level)
{
  p_config->file_log_level = level;
}


/*
 * Sets maximum output level of logging required, has global effect.
 * @param level is the maximum level required - see log_type_t for available
 * options.
 */
void log_set_global_max_output_level(log_type_t level)
{
  global_max_output_level = level;
}


/*
 * Turns logging system on globally.
 */
void log_global_on(void)
{
  log_system_enabled = true;
}


/*
 * Turns logging system off globally.
 */
void log_global_off(void)
{
  log_system_enabled = false;
}


// ------------------------------------------------------------------------- //
// ------------------------ Private Helper Functions ----------------------- //
// ------------------------------------------------------------------------- //

/**
 * Utility function to print labels over serial.
 */
void print_tag_and_log_level(const char *p_tag, log_type_t level)
{
    // Output string buffer. 
    uint8_t msg_buff[30] = {'\0'};

    // Print the system tag.
    strcpy((char*)msg_buff, "\n");
    HAL_UART_Transmit(p_uart_global, msg_buff, 1, TIMEOUT_MS);
    HAL_UART_Transmit(p_uart_global, (const uint8_t*)p_tag,
                    strlen(p_tag), TIMEOUT_MS);

    // Print the log level of the message. 
    if (level == NONE)
    { 
        strcpy((char*)msg_buff, ", ");
        HAL_UART_Transmit(p_uart_global, msg_buff,
                        strlen((const char*)msg_buff), TIMEOUT_MS); 
    } 
    else if (level == WARNING)
    {
        strcpy((char*)msg_buff, ", WARNING: ");
        HAL_UART_Transmit(p_uart_global, msg_buff,
                        strlen((const char*)msg_buff), TIMEOUT_MS);
    } 
    else if (level == INFO)
    {
        strcpy((char*)msg_buff, ", INFO: ");
        HAL_UART_Transmit(p_uart_global, msg_buff,
                        strlen((const char*)msg_buff), TIMEOUT_MS);
    } 
    else if (level == DEBUG)
    {
        strcpy((char*)msg_buff, ", DEBUG: ");
        HAL_UART_Transmit(p_uart_global, msg_buff,
                        strlen((const char*)msg_buff), TIMEOUT_MS);
    } 
    else if (level == VERBOSE_DEBUG)
    {
        strcpy((char*)msg_buff, ", VERBOSE DEBUG: ");
        HAL_UART_Transmit(p_uart_global, msg_buff,
                        strlen((const char*)msg_buff), TIMEOUT_MS);
    } 
    else
    {
        strcpy((char*)msg_buff, ", INVALID_LOG_LEVEL: ");
        HAL_UART_Transmit(p_uart_global, msg_buff,
                        strlen((const char*)msg_buff), TIMEOUT_MS);
    }
}

void print_unsigned_value_with_format(uint32_t val, format_type_t format)
{
    const char hex_prefix[3] = "0x\0";
    const char bin_prefix[3] = "0b\0";
    char num_string[33];

    // Determine format and print value. 
    if (format == DECIMAL)
    {
        // Print decimal value. 
        snprintf(num_string, (DIGITS_IN_DEC_32_T + 1), "%"PRIu32, val);
        HAL_UART_Transmit(p_uart_global, (const uint8_t*)num_string,
                        strlen(num_string), TIMEOUT_MS);
    }
    else if (format == HEXADECIMAL)
    {
        // Print hexadecimal value. 
        snprintf(num_string, (DIGITS_IN_HEX_32_T + 1), "%lx", val);
        HAL_UART_Transmit(p_uart_global, (const uint8_t*)hex_prefix,
                        strlen(hex_prefix), TIMEOUT_MS);
        HAL_UART_Transmit(p_uart_global, (const uint8_t*)num_string,
                        strlen(num_string), TIMEOUT_MS);
    }
    else if (format == BINARY)
    {   
        // Converyt int to binary. 
        for (uint32_t bit = 0; bit < DIGITS_IN_BIN_32_T; ++bit)
        {
            uint32_t mask = (1 << bit);
            if (val & mask)
            {
                num_string[DIGITS_IN_BIN_32_T - bit - 1] = '1';
            }
            else
            {
                num_string[DIGITS_IN_BIN_32_T - bit - 1] = '0';
            }
        }
        // Append null terminator.
        num_string[DIGITS_IN_BIN_32_T] = '\0';        // Print binary value.
        HAL_UART_Transmit(p_uart_global, (const uint8_t*)bin_prefix,
                        strlen(bin_prefix), TIMEOUT_MS);
        HAL_UART_Transmit(p_uart_global, (const uint8_t*)num_string,
                        strlen(num_string), TIMEOUT_MS);
    }
}

void print_signed_value_with_format(int32_t val, format_type_t format)
{
    const char hex_prefix[3] = "0x\0";
    const char bin_prefix[3] = "0b\0";
    const char neg_sign[2] = "-\0";
    const char twos_comp[18] = "(2's Compliment) \0";
    char num_string[33];

    // Determine format and print value. 
    if (format == DECIMAL)
    {
        if (val < 0)
        {
            // Print negative sign.
            HAL_UART_Transmit(p_uart_global, (const uint8_t*)neg_sign,
                        strlen(neg_sign), TIMEOUT_MS);
            // Make val positive.
            val = abs(val);
        }
        // Print decimal value. 
        snprintf(num_string, (DIGITS_IN_DEC_32_T + 1), "%"PRIu32, val);
        HAL_UART_Transmit(p_uart_global, (const uint8_t*)num_string,
                        strlen(num_string), TIMEOUT_MS);
    }
    else if (format == HEXADECIMAL)
    {
        if (val < 0)
        {
            // Print two's compliment reminder.
            HAL_UART_Transmit(p_uart_global, (const uint8_t*)twos_comp,
                        strlen(twos_comp), TIMEOUT_MS);
        }
        // Print hexadecimal value. 
        snprintf(num_string, (DIGITS_IN_HEX_32_T + 1), "%lx", val);
        HAL_UART_Transmit(p_uart_global, (const uint8_t*)hex_prefix,
                        strlen(hex_prefix), TIMEOUT_MS);
        HAL_UART_Transmit(p_uart_global, (const uint8_t*)num_string,
                        strlen(num_string), TIMEOUT_MS);
    }
    else if (format == BINARY)
    {   
        if (val < 0)
        {
            // Print two's compliment reminder.
            HAL_UART_Transmit(p_uart_global, (const uint8_t*)twos_comp,
                        strlen(twos_comp), TIMEOUT_MS);
        }
        // Converyt int to binary. 
        for (uint32_t bit = 0; bit < DIGITS_IN_BIN_32_T; ++bit)
        {
            uint32_t mask = (1 << bit);
            if (val & mask)
            {
                num_string[DIGITS_IN_BIN_32_T - bit - 1] = '1';
            }
            else
            {
                num_string[DIGITS_IN_BIN_32_T - bit - 1] = '0';
            }
        }
        // Append null terminator.
        num_string[DIGITS_IN_BIN_32_T] = '\0';        // Print binary value.
        HAL_UART_Transmit(p_uart_global, (const uint8_t*)bin_prefix,
                        strlen(bin_prefix), TIMEOUT_MS);
        HAL_UART_Transmit(p_uart_global, (const uint8_t*)num_string,
                        strlen(num_string), TIMEOUT_MS);
    }
}


/**
 * Utility function to test if the log message level meets the preferences set.
 */
bool log_message_preference_check(log_system_config_t *p_config,
                                  log_type_t level)
{
  // Test if log_system is enabled globally, if not then do nothing. 
  if (log_system_enabled)
  {
    // Test if the level of this log message meets global log preferences. 
    if (level <= global_max_output_level)
    {
      // Test if the level of this log message meets file level preferences.
      if (level <= p_config->file_log_level)
      {
        return true;
      }
    }
  }
  // If any of the above conditionals evaluate to false, return false. 
  return false;
}

/*** end of file ***/
