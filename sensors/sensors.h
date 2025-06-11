#ifndef SENSORS_H
#define SENSORS_H

#include <stddef.h> // For size_t and other standard definitions
#include <stdint.h> // For uint32_t, uint8_t
#include "vl53l0x.h"
#include <gpio.h>  

extern int i;
extern vl53x sensor;
extern uint32_t baselineDistance;
extern uint32_t currentDistance;
extern uint8_t model, revision;

extern unsigned long red_frequency, green_frequency, blue_frequency, clear_frequency;

// --- Calibration Variables ---
// These will store the pulse counts for white and black surfaces.
extern unsigned long white_red_freq_cal;
extern unsigned long white_green_freq_cal;
extern unsigned long white_blue_freq_cal;
// unsigned long white_clear_freq_cal = 0; // Clear can be useful for auto-brightness adjustment

extern unsigned long black_red_freq_cal;
extern unsigned long black_green_freq_cal;
extern unsigned long black_blue_freq_cal;
// unsigned long black_clear_freq_cal = 0;

int distanceInit();
void colorCalibrate();
void colorInit();
unsigned long getDistance();
char getColor();
unsigned long get_time_us();
unsigned long measure_frequency(io_t pin, unsigned long measurement_time_ms);
void set_filter(char color);
unsigned long map_color(unsigned long x, unsigned long in_min, unsigned long in_max, unsigned long out_min, unsigned long out_max);
char determine_color(unsigned long R, unsigned long G, unsigned long B);

#endif
