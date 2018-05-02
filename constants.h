#ifndef CONSTANTS_H
#define CONSTANTS_H

typedef enum MOTOR_POWER {
    ON = 1,
    OFF = 0,
} MOTOR_POWER;

typedef enum MOTOR_STATE {
    STARTING = 1, // 0b0001,
    RUNNING  = 2, // 0b0010,
    STOPPING = 4, // 0b0100,
    IDLE     = 8, // 0b1000,
} MOTOR_STATE;

typedef enum PANEL {
    STATS = 0,
    HOME = 1,
    SETTINGS = 2,
} PANEL;

#endif // CONSTANTS_H
