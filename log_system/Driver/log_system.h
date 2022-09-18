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
 * @file log_system.h
 * @ingroup log_system
 * @author Jason Duffy
 * @date 18th September 2022
 * @brief Driver file providing logging functionality over UART, to print
 * debug messages and values to a teminal program on your PC. 
 * @bug No known bugs. 
 */

#ifndef LOG_SYSTEM_DOT_H
#define LOG_SYSTEM_DOT_H

#include "stm32f4xx_hal.h"

/**
 * Enumerated constants for the type of message to be logged.
 */
typedef enum
{
    NONE,
    WARNING,
    INFO,
    DEBUG,
    VERBOSE_DEBUG
} log_type_t;


/**
 * Enumerated constants to specify the output format of numerical values.
 */
typedef enum name
{
    DECIMAL,
    HEXADECIMAL,
    BINARY    
} format_type_t;


/**
 * Config object, to be instantiated in each file the log system is to be used,
 * then pass it's address into the functions with names beginning with "log".
 * p_system_tag is a string, and is used for the logsystem to report which file
 * or subsystem the message came from, e.g. "Main".
 * file_log_level is the maximum level you'd like logging output for a
 * particular file.
 */
typedef struct
{
    const char *p_system_tag;
    log_type_t file_log_level;
}log_system_config_t;


/**
 * Initialisation routine - call this function once at startup before using
 * other functions. Log system will then be turned on by default - call
 * log_global_off() if you do not wish it to be.
 */
void init_log_system(UART_HandleTypeDef *p_uart);


/**
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
                 const char *msg);



/**
 * Sends a string, followed by a 64 bit unsigned value (though smaller types
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
                                            format_type_t format);



/**
 * Sends a string, followed by a 64 bit signed value (though smaller types
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
                                            format_type_t format);


/**
 * Sets maximum output level of logging required, to be used at file scope.
 * @param p_config is a pointer to the log_system config object. Instantiate
 * the config object at the head of each file where logging is required and
 * pass it's address into this function.
 * @param level is the maximum level required - see log_type_t for available
 * options.
 */
void log_set_file_max_output_level(log_system_config_t *p_config,
                                   log_type_t level);


/**
 * Sets maximum output level of logging required, has global effect.
 * @param level is the maximum level required - see log_type_t for available
 * options.
 */
void log_set_global_max_output_level(log_type_t level);


/**
 * Turns logging system on globally.
 */
void log_global_on(void);


/**
 * Turns logging system off globally.
 */
void log_global_off(void);

#endif // LOG_SYSTEM_DOT_H