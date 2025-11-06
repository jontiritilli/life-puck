#pragma once

/**
 * @file dice_coin.h
 * @brief Dice rolling and coin flipping functions for tabletop gaming
 * 
 * Provides random number generation for various dice types commonly used
 * in tabletop games, plus coin flipping functionality.
 */

// Dice rolling functions
int roll_d4();   ///< Roll a 4-sided die (1-4)
int roll_d6();   ///< Roll a 6-sided die (1-6) 
int roll_d8();   ///< Roll an 8-sided die (1-8)
int roll_d10();  ///< Roll a 10-sided die (1-10)
int roll_d12();  ///< Roll a 12-sided die (1-12)
int roll_d20();  ///< Roll a 20-sided die (1-20)
int roll_d100(); ///< Roll a 100-sided die (1-100)

// Coin flip function
bool flip_coin(); ///< Flip a coin (returns true for heads, false for tails)

/**
 * @brief Structure to hold dice type information for UI display
 */
struct DiceType {
    const char* name;        ///< Display name of the die (e.g., "d20")
    int (*roll_func)();      ///< Function pointer to the rolling function
};

extern const DiceType DICE_TYPES[];  ///< Array of all available dice types
extern const int DICE_TYPE_COUNT;    ///< Number of dice types available
