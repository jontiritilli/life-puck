#pragma once
#include <Wire.h> 
#include "board_config.h"  // Include board-specific pin definitions

/**
 * @brief Initialize I2C bus for peripheral communication
 * 
 * Configures I2C pins and starts Wire library for communication
 * with touch controller, GPIO expander, and other peripherals.
 * Uses pin definitions from board_config.h to support different hardware variants.
 */
void I2C_Init(void);

/**
 * @brief Read data from I2C device register
 * @param Driver_addr I2C device address (7-bit)
 * @param Reg_addr Register address to read from
 * @param Reg_data Buffer to store read data
 * @param Length Number of bytes to read
 * @return true if read successful, false on error
 */
bool I2C_Read(uint8_t Driver_addr, uint8_t Reg_addr, uint8_t *Reg_data, uint32_t Length);

/**
 * @brief Write data to I2C device register
 * @param Driver_addr I2C device address (7-bit)
 * @param Reg_addr Register address to write to
 * @param Reg_data Data buffer to write
 * @param Length Number of bytes to write
 * @return true if write successful, false on error
 */
bool I2C_Write(uint8_t Driver_addr, uint8_t Reg_addr, const uint8_t *Reg_data, uint32_t Length);