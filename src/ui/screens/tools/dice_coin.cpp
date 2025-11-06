#include "dice_coin.h"
#include <esp_random.h>

int roll_d4() { 
    return (esp_random() % 4) + 1; 
}

int roll_d6() { 
    return (esp_random() % 6) + 1; 
}

int roll_d8() { 
    return (esp_random() % 8) + 1; 
}

int roll_d10() { 
    return (esp_random() % 10) + 1; 
}

int roll_d12() { 
    return (esp_random() % 12) + 1; 
}

int roll_d20() { 
    return (esp_random() % 20) + 1; 
}

int roll_d100() { 
    return (esp_random() % 100) + 1; 
}

bool flip_coin() { 
    return (esp_random() % 2) == 0; 
}

// Würfel-Liste für UI
const DiceType DICE_TYPES[] = {
    {"D4", roll_d4},
    {"D6", roll_d6},
    {"D8", roll_d8},
    {"D10", roll_d10},
    {"D12", roll_d12},
    {"D20", roll_d20},
    {"D100", roll_d100}
};

const int DICE_TYPE_COUNT = 7;
